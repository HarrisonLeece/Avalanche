#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "thread_pool.hpp"
#include "matplot/matplot.h"
#include <aubio/aubio.h>
#include <cmath>
#include <assert.h>
//#include "vectorUtils.cpp"
#include "pocketfft_hdronly.h"


class energyBands
{
  public:

    int numBands = 4; // How many energy bands to divide the fft into

    vector<int> energyMem_size; //Keeps the energy memory length for each frequency range
    vector<vector<double>> energyMem; //energyMem.size() = numBands
    vector<vector<double>> energyDeriv; //Differentiate energyMem and load derivative into (not implemented 2/4/22)
    vector<double> moving_avg; //

    vector<double> variance; //The multiple by which a currentEnergy must exceed energyMem in order to be known as a beat
    vector<double> currentEnergy; // initialize empty, hold the most recently calculated energy quantity from the PSD

    vector<vector<int>> freq_range; //Vector of vectors which contain the index pair for PSD slicing

    twp *tmWvPwr;
    int fps = 60;

    bool first_psd = true;

    bool remove_detected_energy = false;

    energyBands(twp &timeWavePower);
    ~energyBands();
    void setTWP(twp &timeWavePower);//Do first thing after intialization if not constructed with argument
    void add_freq_range(vector<vector<int>> freq_range_in);
    vector<int> detectBeat(int frame); //returns a vector of size numBands populated by an integer, based on which band a beat was detected
  private:
    void conditionPSD(vector<float> &onePSD); //Break onePSD into numBands bands and push back energyBandVec with a vector of length numBands
    void set_moving_avg(int band);

};

energyBands::energyBands(twp &timeWavePower) {
  tmWvPwr = &timeWavePower;
}

void energyBands::setTWP(twp &timeWavePower) {
  //The memory address of the twp object enters the function, and then member variable
  tmWvPwr = &timeWavePower;
  fps = tmWvPwr->fps;
}

energyBands::~energyBands() {
  //tmWvPwr was not assigned through malloc therefore a memory free() will crash the program
  //delete tmWvPwr;
}

void energyBands::add_freq_range(vector<vector<int>> freq_range_in){
  //freq_range_in comes in as
  numBands = freq_range_in.size();
  vector<int> range_pair(2,0); // size 2, initialized to 0
  vector<int> rng;
  for (int r = 0; r < freq_range_in.size(); r++){
    rng = freq_range_in[r];
    range_pair[0] = rng[0];
    range_pair[1] = rng[1];
    energyMem_size.push_back(rng[2]);
    freq_range.push_back(range_pair);
    variance.push_back(rng[3]/100);
  }
}

//After this function exits, energyMem and current energy should be updated to the current frame's data
void energyBands::conditionPSD(vector<float> &psd)
{
  int psd_size = psd.size();
  //This only runs once when beat detection start
  int samplefreq = tmWvPwr->sampFreq;
  if (first_psd) {
    //The 2*psd_size is because I've already trimmed the PSD of opposite phase components before saving to twp
    double freq_per_bin = samplefreq/(2*psd_size);
    //Convert frequency ranges to indexes using psd size and sample rate
    for (int pair_index = 0; pair_index < freq_range.size(); pair_index++){
      freq_range[pair_index][0] = (int) round(freq_range[pair_index][0]/freq_per_bin);
      freq_range[pair_index][1] = (int) round(freq_range[pair_index][1]/freq_per_bin);
    }
    //setup energy memory with an empty vector
    vector<double> zeros(1,0);
    moving_avg.push_back(0);
    for (int band = 0; band < numBands; band++){
      currentEnergy.push_back(0);
      energyMem.push_back(zeros);
      energyDeriv.push_back(zeros);
    }

    first_psd = false;
  }
  assert(currentEnergy.size() == numBands);

  vector<float> tempVec; //holds the psd data points
  double energy;

  //Slice input power spectrum, sum into energy (per band), add energy per band into
  //the energyBandVec. Place current energy into currentEnergy vector
  for (int i=0; i < numBands; i++)
  {
    tempVec = sliceVectorFloat(psd,freq_range[i][0], freq_range[i][1]); //slice PSD
    int band_length = freq_range[i][1] - freq_range[i][0];
    //find the total energy https://en.wikipedia.org/wiki/Parseval%27s_theorem
    energy = (2.0/band_length)*sumVector<float>(tempVec); //Again the 2.0 multiple is from symetriccal portion of the FFT being cut out
    energyMem[i].push_back(energy);
    currentEnergy[i]=energy;
    //Differentiate here
    if (energyMem.size()>1){
      energyDeriv[i].push_back((energy - (energyMem[i][energyMem[i].size()-2]))/fps);
    } else {
      energyDeriv[i].push_back(0);
    }
  }
}

void energyBands::set_moving_avg(int band){
  //subtract one because end() is the past-the-end index, and my slicing function is inclusive of the end index
  int size = energyMem[band].size();
  if (size>energyMem_size[band]){
    vector<double> slicedMem = sliceVectorDouble(energyMem[band], size-energyMem_size[band]-1, size-1);
    moving_avg[band] = sumVector<double>(slicedMem)/energyMem_size[band];
  } else {
    moving_avg[band] = sumVector<double>(energyMem[band])/size;
  }
}

//returns a vector of length numBands
//The integer returned will be equal to the subBand which triggered
/*  Before a beat is detection, a PSD at the index of the frame number passed is 'processed' or 'conditioned'
//  After 'conditioning', the energy memory will be adjusted, and currentEnergy will be up to date, but the moving average is not affected until
//  setEnergMemory() is called.
*/
vector<int> energyBands::detectBeat(int frame)
{
  vector<int> out; // return vector
  conditionPSD(tmWvPwr->psdMetaVector[frame]); // now current energy represents the input frame
  bool deriv = true; // if we use the derivative method
  for (int band = 0; band < numBands; band++)
  {

    if(!deriv){
      //We also need to pop the energyBandVec so that multiple beats within the energyHistory window can still be detected without error
      if (currentEnergy[band] > moving_avg[band] * variance[band] && moving_avg[band] != 0)
      {
        //Load our out vector with an integer, indicating beat detection
        out.push_back(band+1);//This is plus 1, so that index 0 becomes 1 in the out vector

        if (remove_detected_energy){
          energyMem[band].pop_back(); //remove the current beat's energy from energyMem
          energyMem[band].push_back(energyMem[band][(energyMem[band].size()-2)]); //Replace with a previous energy value with a value in energyMem
        }

        set_moving_avg(band); //Set energy memory
      } else {
        out.push_back(0);
        set_moving_avg(band);
      }
    } else if (deriv) {
      if (energyDeriv[band].back() > variance[band]) {
        cout << energyDeriv[band].back() << endl;
        //Load our out vector with an integer, indicating beat detection
        out.push_back(band+1);//This is plus 1, so that index 0 becomes 1 in the out vector
        set_moving_avg(band); //Set energy memory
      }else {
        cout << energyDeriv[band].back() << endl;
        out.push_back(0);
        set_moving_avg(band);
      }
    }

  }
  assert(out.size() == numBands);
  return out;
}
