#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>

class flg {
  public:
    string path = "../flag_files/";
    string file = "output.flg";

    vector<int> waveform;
    vector<vector<float>> psdMetaVector;
    int fps=60;
    int sampFreq=44100;
    int waveLength=-97;
    int numPSD=-1337;
    int psdSize=0;
    int startPadding=-69;
    int endPadding=-69;

    vector<vector<int>> beatMatrix;
    int bmSize = -420;
    vector<float> filt_vector;



    //
    vector<double> times; //Vector of times starting at 0
    vector<int> frames; //Vector of frames starting at 0
    vector<vector<int>> beatsVec; //vector of beats vectors

    //For encoding arbitrary flags
    int numFlags; //Number of channels containing data

    //i/o
    void writeFLG(string out_name);
    void readFLG(string in_name);


    void addPSDtoFLG(vector<vector<float>> psdMatrix);
    void addWavetoFLG(vector<int> wave);
    void importTWP(vector<float> singlePSD);
    void addBeatsToFLG(vector<vector<int>> beatMat);
    void setFPSandSampFreq(int framesPer, int Fs);
    //This flters the beatMatrix member variable to indicate exactly one start position per beat
    void beatCleanup(vector<float> &filt_params);


};


void flg::writeFLG(string out_name="output.flg"){
  file = out_name;
  string outputFilePath = path + file;
  cout << "Writing FLG file to " << outputFilePath << endl;

  ofstream targetFLG (outputFilePath);
  if (targetFLG.is_open())
  {
    //Write any header data
    targetFLG << fps << "\n";
    targetFLG << sampFreq << "\n";

    if (numPSD != -1337) {
      startPadding = ceil((psdSize/2)/((float)(sampFreq/fps)));
      endPadding = startPadding;

      targetFLG << startPadding << '\n';
      targetFLG << endPadding << '\n';
    } else {
      targetFLG << startPadding << '\n';
      targetFLG << endPadding << '\n';
    }

    targetFLG << waveLength << '\n';
    targetFLG << numPSD << '\n';
    targetFLG << bmSize << '\n';

    if (bmSize != -420){
      //write beat matrix
      for (int row = 0; row < bmSize; row++){
        for (int col = 0;  col < beatMatrix[row].size() ; col++)
        {
          targetFLG << beatMatrix[row][col] << ',';
        }
        targetFLG << '\n';
      }
      targetFLG << "#EndBeatMatrix" << '\n';
    }
    if (waveLength != -97) {
      //write wave data
      //Put waveform into the file
      for (int i = 0;  i < waveLength; i++)
      {
        targetFLG << waveform[i] << ',';
      }
      targetFLG << '\n';
      targetFLG << "#EndWaveform" << '\n';
    }
    if (numPSD != -1337){
      //write psd data
      for (int i=0; i<numPSD; i++){
        for (int k = 0;  k < psdSize; k++)
        {
          targetFLG << psdMetaVector[i][k] << ',';
        }
        if (i < numPSD-1){
          targetFLG << '\n';
        }
      }
      targetFLG << '\n' << "#EndPSDs";
    }
  }
  targetFLG.close();
}

void flg::readFLG(string in_name="output.flg"){
  file = in_name;
  string inputFilePath = path + file;
  cout << "Reading FLG file from " << inputFilePath << endl;

  ifstream targetFLG (inputFilePath);
  if (!targetFLG.is_open())
  {
      fprintf(stderr, "Unable to open FLG file: %s\n", inputFilePath);
      return;
  }
  string headerTemp;
  //Read in the HEADER
  //strictly speaking, not necessary, but I like it to make the twp file human readable
  getline(targetFLG, headerTemp);
  fps = stoi(headerTemp);
  getline(targetFLG, headerTemp);
  sampFreq = stoi(headerTemp);
  getline(targetFLG, headerTemp);
  startPadding = stoi(headerTemp);
  getline(targetFLG, headerTemp);
  endPadding = stoi(headerTemp);
  getline(targetFLG, headerTemp);
  waveLength = stoi(headerTemp);
  getline(targetFLG, headerTemp);
  numPSD = stoi(headerTemp);
  getline(targetFLG, headerTemp);
  bmSize = stoi(headerTemp);

  string delimiter =",";
  string end_string;
  //initialize pos which is the location of the delimeter

  if (bmSize != -420) {
    for (string line; getline(targetFLG, line);) {
      istringstream ss0(line);
      if (line == "#EndBeatMatrix") {
        end_string = line;
        break;
      }
      vector<int> tmpBts;
      for (float i; ss0 >> i;){
        tmpBts.push_back(i);
        if (ss0.peek() == ','){
          ss0.ignore();
        }
      }
      beatMatrix.push_back(tmpBts);
    }
    bmSize = beatMatrix.size();
    cout << "Beat matrix read" << endl;
    assert (end_string == "#EndBeatMatrix");
  }
  if (waveLength != -97) {
    int ssInt;
    string unparsedWave;
    //if I understand std::getline correctly; should erase the extracted
    //line after putting the line into argument 2 (unparsedWave)
    getline(targetFLG, unparsedWave);
    istringstream ss(unparsedWave);
    //https://stackoverflow.com/questions/1894886/parsing-a-comma-delimited-stdstring
    for (int i; ss >> i;){
      waveform.push_back(i);
      if (ss.peek() == ','){
        ss.ignore();
      }
    }
    cout << "Waveform read" << endl;
    getline(targetFLG, end_string);
    cout << end_string << endl << flush;
    assert (end_string == "#EndWaveform");
  }
  if (numPSD != -1337) {
    for (string line; getline(targetFLG, line);) {
      istringstream ss2(line);
      if (line == "#EndPSDs") {
        break;
      }
      vector<float> tmpPSD;
      for (float i; ss2 >> i;){
        tmpPSD.push_back(i);
        if (ss2.peek() == ','){
          ss2.ignore();
        }
      }
      psdMetaVector.push_back(tmpPSD);
    }
    numPSD = psdMetaVector.size();
    psdSize = psdMetaVector.back().size();
    cout << "PSDs read" << endl;
  }
  targetFLG.close();
}

void flg::setFPSandSampFreq(int framesPer, int Fs) {
  sampFreq = Fs;
  fps = framesPer;
  if (psdSize != 0) {
    int tmp = ceil((psdSize/2)/(sampFreq/fps));
    startPadding = tmp;
    endPadding = tmp;
  }
}

void flg::importTWP(vector<float> singlePSD){
  cout << "not implemented" << endl;
}

void flg::addWavetoFLG(vector<int> wave){
  waveform = wave;
  waveLength = wave.size();
}

void flg::addPSDtoFLG(vector<vector<float>> psdMatrix) {
  psdMetaVector = psdMatrix;
  numPSD = psdMatrix.size();
  psdSize = psdMatrix[0].size();

}

void flg::addBeatsToFLG(vector<vector<int>> beatMat){
  beatMatrix=beatMat;
  bmSize = beatMatrix.size();
}


void flg::beatCleanup(vector<float> &filt_params) {
  assert(filt_params.size() == beatMatrix[0].size());
  int param;
  vector <int> int_params;
  for (int k = 0; k < filt_params.size(); k++){
    int_params.push_back((int)round(filt_params[k]*60));
  }
  vector<int> skip_counter(filt_params.size(),0);
  for (int frame = 0; frame < beatMatrix.size(); frame++) {
    for (int k = 0; k < beatMatrix[0].size(); k++){
      if (beatMatrix[frame][k] != 0 && skip_counter[k]==0) {
        skip_counter[k]=int_params[k];
      } else {
        if (skip_counter[k] != 0) {
          beatMatrix[frame][k] = 0;
          skip_counter[k] = skip_counter[k] - 1;
        }
      }
    }
  }
}
