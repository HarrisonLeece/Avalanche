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
#include <math.h>
#include "vectorUtils.cpp"

using namespace std;


///////
// Plot waveforms or arbitrary vectors
///////
void plot_mono_channel(vector<int> channel)
{
    using namespace matplot;
    int length = channel.size();
    cout << "Running mono plotter" << endl;


    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }
    try {
      plot(x, channel)->color({0.f, 0.7f, 0.9f});
      title("Single Channel Plot");
      xlabel("# samples");
      ylabel("Channel values");
    } catch (const runtime_error& e) {
      cout << "plot_stereo_channel; An error occured while plotting the graphs" << endl;
      cout << "sample indexes (variable x)" << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
    }
    show();
}

void test_psds(vector<float> channel)
{
    using namespace matplot;
    //Slice the incoming vector to remove the 0Hz bin
    //channel = sliceVectorFloat(channel, 5); //remove index 0 to index 4 (5 total indexes removed)

    int length = channel.size();
    cout << "Running psd_tester" << endl;

    vector<float> nat_log;
    //take the natural logorithm of channel data
    for (int i = 0; i < channel.size(); i++){
      nat_log.push_back(log(channel[i]));
    }

    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }
    try {
      plot(x, nat_log)->color({7.f, 0.0f, 0.9f});
      title("PSD ln Plot");
      xlabel("# samples");
      ylabel("Channel (log) values");
    } catch (const runtime_error& e) {
      cout << "test_psds; An error occured while plotting the graphs" << endl;
      cout << "sample indexes (variable x)" << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
      cout << endl;
      for (int i=0; i < length; i++){
        cout << nat_log[i] << ", ";
      }

    }
    show();
}

void plot_detected_beats(vector<vector<int>> beatsVec){
  using namespace matplot;
  vector<double> x;
  vector<double> y;
  int numframes = beatsVec.size();
  int numbands = beatsVec[0].size();
  for (int frame=0; frame<numframes; frame++) {
    for (int band=0; band<numbands; band++){
      if (beatsVec[frame][band] != 0) {
        x.push_back(frame/60);
        y.push_back(band+1);
      }
    }
  }
  scatter(x, y);
  show();
}

void plot_stereo_channel(vector<int> left_channel, vector<int> right_channel)
{
    using namespace matplot;
    cout << "Running stereo plotter" << endl;
    int length = left_channel.size();
    if (length != right_channel.size()){
      cout << "Error in plot_stereo_channel; left and right channels size mismatch" << endl;
      cout << "left channel: "  << length << " samples; right channel: " << right_channel.size() << " samples" << endl;
      return;
    }
    //vector of integers, x, should be the length of the number of samples starting at zero
    vector<double> x = linspace(0, length-1, length);
    //print_vector_to_console_double(x);
    //In case we find out we have to cast the vector of doubles to int...
    //vector<int> x((length));
    //for (int i = 0; i < length; i++){
    //  x.at(i) = int (tmp[i]);
    //}
    //print_vector_to_console_int(x);
    try {
      hold(on);
      plot(x, left_channel)->color({0.f, 0.7f, 0.9f});
      plot(x,right_channel)->color({0.9f, 0.f, 0.1f});
      hold(off);
      title("Stereo Channels Plot");
      xlabel("# samples");
      ylabel("Channel values");
    } catch (const runtime_error& e) {
      cout << "plot_stereo_channel; An error occured while plotting the graphs" << endl;
      cout << "sample indexes (variable x)" << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      cout << endl << "left channel values (variable left_channel)" << endl;
      for (int i=0; i < length; i++){
        cout << left_channel[i] << ", ";
      }
      cout << endl << "right channel values (variable right_channel)" << endl;
      for (int i=0; i < length; i++){
        cout << right_channel[i] << ", ";
      }
    }
    show();
}
//


///////
// Helper functions for plotting simple vectors of numbers
///////
void plot_vector_double(vector<double> in_vec){
  using namespace matplot;
  int length = in_vec.size();
  vector<double> x = linspace(0, length-1, length);

  plot(x, in_vec)->color({0.f, 0.7f, 0.9f});
  show();
}
void plot_vector_int(vector<int> in_vec){
  using namespace matplot;
  int length = in_vec.size();
  vector<double> x = linspace(0, length-1, length);

  plot(x, in_vec)->color({0.f, 0.7f, 0.9f});
  show();
}
////////
