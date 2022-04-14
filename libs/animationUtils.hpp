#pragma once
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "matplot/matplot.h"
//#include <SFML/Audio.hpp>

void animate_single_channel(vector<int> channel, auto anifig, int index, string flname = "waveform", vector<float> clr = {0.f,0.7f,1.0f} )
{
    using namespace matplot;
    //anifig->clear(); // clear the plot
    int length = channel.size();
    cout << "Running single channel animator" << endl;

    //create suitable x axis linspace
    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }

    try {
      auto p = plot(x, channel)->color({clr[0],clr[1],clr[2]}); //blue
      p.line_width(8);
      //title("Single Channel Plot");
      //xlabel("Sample");
      //ylabel("Channel values");

    } catch (const runtime_error& e) {
      cout << "animate_single_channel; An error occured while plotting the graphs" << endl;
      //print the x axis
      cout << "sample indexes (variable x): " << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      //print the y axis
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
    }
    string fileName = "./plot_saves/"+flname;
    fileName = fileName + to_string(index);
    save(fileName,"svg");
    anifig->draw();
}

void animate_single_channelFloat(vector<float> channel, auto anifig, int index, string flname = "waveform", vector<float> clr = {1.f, 0.0625f, 0.9375f} )
{
    using namespace matplot;
    //anifig->clear(); // clear the plot
    int length = channel.size();
    cout << "Running single channel animator" << endl;

    //create suitable x axis linspace
    vector<double> tmp = linspace(0, length-1, length);
    vector<int> x(length);
    for (int i=0; i < length; i++){
      x[i]=int (tmp[i]);
    }

    try {
      auto p = plot(x, channel)->color({clr[0],clr[1],clr[2]}); //pink
      p.line_width(8);
      //title("Single Channel Plot");
      //xlabel("Sample");
      //ylabel("Channel values");

    } catch (const runtime_error& e) {
      cout << "animate_single_channel; An error occured while plotting the graphs" << endl;
      //print the x axis
      cout << "sample indexes (variable x): " << endl;
      for (int i=0; i < length; i++){
        cout << x[i] << ", ";
      }
      //print the y axis
      cout << endl << "single channel values (variable channel)" << endl;
      for (int i=0; i < length; i++){
        cout << channel[i] << ", ";
      }
    }
    string fileName = "./plot_saves/"+flname;
    fileName = fileName + to_string(index);
    save(fileName,"svg");
    anifig->draw();
}

void animate_beat_vec(auto anifig, vector<double> &x,vector<double> &y, int frame_num, int numbands){
  using namespace matplot;
  auto animationFigure = anifig;
  animationFigure -> size(1600,1000);
  double frame_time=frame_num/60.0;

  vector<double> fakex;
  vector<double> fakey;
  for (int k=0;k<numbands*2;k++){
    fakex.push_back(frame_time);
    fakey.push_back(k/2.0);
  }
  hold(on);
  auto p = scatter(fakex,fakey)->color({1.0f,0.1f,0.1f}); //red
  //auto a = scatter(x,y);

  animationFigure->draw();

  string fileName = "./beat_detection_graph/";
  fileName = fileName + to_string(frame_num);
  save(fileName,"png");
  hold(off);
}
