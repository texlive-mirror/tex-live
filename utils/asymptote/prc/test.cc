/************
*
*   This file is part of a tool for producing 3D content in the PRC format.
*   Copyright (C) 2008  Orest Shardt <shardtor (at) gmail dot com>
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include "oPRCFile.h"

using namespace std;

int main()
{
//  ofstream outf("test.prc",ios::binary);
  double knotsU[] = {1,1,1,1,2,2,2,2};
  double knotsV[] = {1,1,1,1,2,2,2,2};
  const int NUMBER_OF_PATCHES = 32;
  double controlPoints[NUMBER_OF_PATCHES][16][3] = 
  {
    { // Patch 0
      {1.4,0,2.4},{1.4,-0.784,2.4},{0.784,-1.4,2.4},{0,-1.4,2.4},
      {1.3375,0,2.53125},{1.3375,-0.749,2.53125},{0.749,-1.3375,2.53125},{0,-1.3375,2.53125},
      {1.4375,0,2.53125},{1.4375,-0.805,2.53125},{0.805,-1.4375,2.53125},{0,-1.4375,2.53125},
      {1.5,0,2.4},{1.5,-0.84,2.4},{0.84,-1.5,2.4},{0,-1.5,2.4},
    },
    { // Patch 1
      {0,-1.4,2.4},{-0.784,-1.4,2.4},{-1.4,-0.784,2.4},{-1.4,0,2.4},
      {0,-1.3375,2.53125},{-0.749,-1.3375,2.53125},{-1.3375,-0.749,2.53125},{-1.3375,0,2.53125},
      {0,-1.4375,2.53125},{-0.805,-1.4375,2.53125},{-1.4375,-0.805,2.53125},{-1.4375,0,2.53125},
      {0,-1.5,2.4},{-0.84,-1.5,2.4},{-1.5,-0.84,2.4},{-1.5,0,2.4},
    },
    { // Patch 2
      {-1.4,0,2.4},{-1.4,0.784,2.4},{-0.784,1.4,2.4},{0,1.4,2.4},
      {-1.3375,0,2.53125},{-1.3375,0.749,2.53125},{-0.749,1.3375,2.53125},{0,1.3375,2.53125},
      {-1.4375,0,2.53125},{-1.4375,0.805,2.53125},{-0.805,1.4375,2.53125},{0,1.4375,2.53125},
      {-1.5,0,2.4},{-1.5,0.84,2.4},{-0.84,1.5,2.4},{0,1.5,2.4},
    },
    { // Patch 3
      {0,1.4,2.4},{0.784,1.4,2.4},{1.4,0.784,2.4},{1.4,0,2.4},
      {0,1.3375,2.53125},{0.749,1.3375,2.53125},{1.3375,0.749,2.53125},{1.3375,0,2.53125},
      {0,1.4375,2.53125},{0.805,1.4375,2.53125},{1.4375,0.805,2.53125},{1.4375,0,2.53125},
      {0,1.5,2.4},{0.84,1.5,2.4},{1.5,0.84,2.4},{1.5,0,2.4},
    },
    { // Patch 4
      {1.5,0,2.4},{1.5,-0.84,2.4},{0.84,-1.5,2.4},{0,-1.5,2.4},
      {1.75,0,1.875},{1.75,-0.98,1.875},{0.98,-1.75,1.875},{0,-1.75,1.875},
      {2,0,1.35},{2,-1.12,1.35},{1.12,-2,1.35},{0,-2,1.35},
      {2,0,0.9},{2,-1.12,0.9},{1.12,-2,0.9},{0,-2,0.9},
    },
    { // Patch 5
      {0,-1.5,2.4},{-0.84,-1.5,2.4},{-1.5,-0.84,2.4},{-1.5,0,2.4},
      {0,-1.75,1.875},{-0.98,-1.75,1.875},{-1.75,-0.98,1.875},{-1.75,0,1.875},
      {0,-2,1.35},{-1.12,-2,1.35},{-2,-1.12,1.35},{-2,0,1.35},
      {0,-2,0.9},{-1.12,-2,0.9},{-2,-1.12,0.9},{-2,0,0.9},
    },
    { // Patch 6
      {-1.5,0,2.4},{-1.5,0.84,2.4},{-0.84,1.5,2.4},{0,1.5,2.4},
      {-1.75,0,1.875},{-1.75,0.98,1.875},{-0.98,1.75,1.875},{0,1.75,1.875},
      {-2,0,1.35},{-2,1.12,1.35},{-1.12,2,1.35},{0,2,1.35},
      {-2,0,0.9},{-2,1.12,0.9},{-1.12,2,0.9},{0,2,0.9},
    },
    { // Patch 7
      {0,1.5,2.4},{0.84,1.5,2.4},{1.5,0.84,2.4},{1.5,0,2.4},
      {0,1.75,1.875},{0.98,1.75,1.875},{1.75,0.98,1.875},{1.75,0,1.875},
      {0,2,1.35},{1.12,2,1.35},{2,1.12,1.35},{2,0,1.35},
      {0,2,0.9},{1.12,2,0.9},{2,1.12,0.9},{2,0,0.9},
    },
    { // Patch 8
      {2,0,0.9},{2,-1.12,0.9},{1.12,-2,0.9},{0,-2,0.9},
      {2,0,0.45},{2,-1.12,0.45},{1.12,-2,0.45},{0,-2,0.45},
      {1.5,0,0.225},{1.5,-0.84,0.225},{0.84,-1.5,0.225},{0,-1.5,0.225},
      {1.5,0,0.15},{1.5,-0.84,0.15},{0.84,-1.5,0.15},{0,-1.5,0.15},
    },
    { // Patch 9
      {0,-2,0.9},{-1.12,-2,0.9},{-2,-1.12,0.9},{-2,0,0.9},
      {0,-2,0.45},{-1.12,-2,0.45},{-2,-1.12,0.45},{-2,0,0.45},
      {0,-1.5,0.225},{-0.84,-1.5,0.225},{-1.5,-0.84,0.225},{-1.5,0,0.225},
      {0,-1.5,0.15},{-0.84,-1.5,0.15},{-1.5,-0.84,0.15},{-1.5,0,0.15},
    },
    { // Patch 10
      {-2,0,0.9},{-2,1.12,0.9},{-1.12,2,0.9},{0,2,0.9},
      {-2,0,0.45},{-2,1.12,0.45},{-1.12,2,0.45},{0,2,0.45},
      {-1.5,0,0.225},{-1.5,0.84,0.225},{-0.84,1.5,0.225},{0,1.5,0.225},
      {-1.5,0,0.15},{-1.5,0.84,0.15},{-0.84,1.5,0.15},{0,1.5,0.15},
    },
    { // Patch 11
      {0,2,0.9},{1.12,2,0.9},{2,1.12,0.9},{2,0,0.9},
      {0,2,0.45},{1.12,2,0.45},{2,1.12,0.45},{2,0,0.45},
      {0,1.5,0.225},{0.84,1.5,0.225},{1.5,0.84,0.225},{1.5,0,0.225},
      {0,1.5,0.15},{0.84,1.5,0.15},{1.5,0.84,0.15},{1.5,0,0.15},
    },
    { // Patch 12
      {-1.6,0,2.025},{-1.6,-0.3,2.025},{-1.5,-0.3,2.25},{-1.5,0,2.25},
      {-2.3,0,2.025},{-2.3,-0.3,2.025},{-2.5,-0.3,2.25},{-2.5,0,2.25},
      {-2.7,0,2.025},{-2.7,-0.3,2.025},{-3,-0.3,2.25},{-3,0,2.25},
      {-2.7,0,1.8},{-2.7,-0.3,1.8},{-3,-0.3,1.8},{-3,0,1.8},
    },
    { // Patch 13
      {-1.5,0,2.25},{-1.5,0.3,2.25},{-1.6,0.3,2.025},{-1.6,0,2.025},
      {-2.5,0,2.25},{-2.5,0.3,2.25},{-2.3,0.3,2.025},{-2.3,0,2.025},
      {-3,0,2.25},{-3,0.3,2.25},{-2.7,0.3,2.025},{-2.7,0,2.025},
      {-3,0,1.8},{-3,0.3,1.8},{-2.7,0.3,1.8},{-2.7,0,1.8},
    },
    { // Patch 14
      {-2.7,0,1.8},{-2.7,-0.3,1.8},{-3,-0.3,1.8},{-3,0,1.8},
      {-2.7,0,1.575},{-2.7,-0.3,1.575},{-3,-0.3,1.35},{-3,0,1.35},
      {-2.5,0,1.125},{-2.5,-0.3,1.125},{-2.65,-0.3,0.9375},{-2.65,0,0.9375},
      {-2,0,0.9},{-2,-0.3,0.9},{-1.9,-0.3,0.6},{-1.9,0,0.6},
    },
    { // Patch 15
      {-3,0,1.8},{-3,0.3,1.8},{-2.7,0.3,1.8},{-2.7,0,1.8},
      {-3,0,1.35},{-3,0.3,1.35},{-2.7,0.3,1.575},{-2.7,0,1.575},
      {-2.65,0,0.9375},{-2.65,0.3,0.9375},{-2.5,0.3,1.125},{-2.5,0,1.125},
      {-1.9,0,0.6},{-1.9,0.3,0.6},{-2,0.3,0.9},{-2,0,0.9},
    },
    { // Patch 16
      {1.7,0,1.425},{1.7,-0.66,1.425},{1.7,-0.66,0.6},{1.7,0,0.6},
      {2.6,0,1.425},{2.6,-0.66,1.425},{3.1,-0.66,0.825},{3.1,0,0.825},
      {2.3,0,2.1},{2.3,-0.25,2.1},{2.4,-0.25,2.025},{2.4,0,2.025},
      {2.7,0,2.4},{2.7,-0.25,2.4},{3.3,-0.25,2.4},{3.3,0,2.4},
    },
    { // Patch 17
      {1.7,0,0.6},{1.7,0.66,0.6},{1.7,0.66,1.425},{1.7,0,1.425},
      {3.1,0,0.825},{3.1,0.66,0.825},{2.6,0.66,1.425},{2.6,0,1.425},
      {2.4,0,2.025},{2.4,0.25,2.025},{2.3,0.25,2.1},{2.3,0,2.1},
      {3.3,0,2.4},{3.3,0.25,2.4},{2.7,0.25,2.4},{2.7,0,2.4},
    },
    { // Patch 18
      {2.7,0,2.4},{2.7,-0.25,2.4},{3.3,-0.25,2.4},{3.3,0,2.4},
      {2.8,0,2.475},{2.8,-0.25,2.475},{3.525,-0.25,2.49375},{3.525,0,2.49375},
      {2.9,0,2.475},{2.9,-0.15,2.475},{3.45,-0.15,2.5125},{3.45,0,2.5125},
      {2.8,0,2.4},{2.8,-0.15,2.4},{3.2,-0.15,2.4},{3.2,0,2.4},
    },
    { // Patch 19
      {3.3,0,2.4},{3.3,0.25,2.4},{2.7,0.25,2.4},{2.7,0,2.4},
      {3.525,0,2.49375},{3.525,0.25,2.49375},{2.8,0.25,2.475},{2.8,0,2.475},
      {3.45,0,2.5125},{3.45,0.15,2.5125},{2.9,0.15,2.475},{2.9,0,2.475},
      {3.2,0,2.4},{3.2,0.15,2.4},{2.8,0.15,2.4},{2.8,0,2.4},
    },
    { // Patch 20
      {0,0,3.15},{0,0,3.15},{0,0,3.15},{0,0,3.15},
      {0.8,0,3.15},{0.8,-0.45,3.15},{0.45,-0.8,3.15},{0,-0.8,3.15},
      {0,0,2.85},{0,0,2.85},{0,0,2.85},{0,0,2.85},
      {0.2,0,2.7},{0.2,-0.112,2.7},{0.112,-0.2,2.7},{0,-0.2,2.7},
    },
    { // Patch 21
      {0,0,3.15},{0,0,3.15},{0,0,3.15},{0,0,3.15},
      {0,-0.8,3.15},{-0.45,-0.8,3.15},{-0.8,-0.45,3.15},{-0.8,0,3.15},
      {0,0,2.85},{0,0,2.85},{0,0,2.85},{0,0,2.85},
      {0,-0.2,2.7},{-0.112,-0.2,2.7},{-0.2,-0.112,2.7},{-0.2,0,2.7},
    },
    { // Patch 22
      {0,0,3.15},{0,0,3.15},{0,0,3.15},{0,0,3.15},
      {-0.8,0,3.15},{-0.8,0.45,3.15},{-0.45,0.8,3.15},{0,0.8,3.15},
      {0,0,2.85},{0,0,2.85},{0,0,2.85},{0,0,2.85},
      {-0.2,0,2.7},{-0.2,0.112,2.7},{-0.112,0.2,2.7},{0,0.2,2.7},
    },
    { // Patch 23
      {0,0,3.15},{0,0,3.15},{0,0,3.15},{0,0,3.15},
      {0,0.8,3.15},{0.45,0.8,3.15},{0.8,0.45,3.15},{0.8,0,3.15},
      {0,0,2.85},{0,0,2.85},{0,0,2.85},{0,0,2.85},
      {0,0.2,2.7},{0.112,0.2,2.7},{0.2,0.112,2.7},{0.2,0,2.7},
    },
    { // Patch 24
      {0.2,0,2.7},{0.2,-0.112,2.7},{0.112,-0.2,2.7},{0,-0.2,2.7},
      {0.4,0,2.55},{0.4,-0.224,2.55},{0.224,-0.4,2.55},{0,-0.4,2.55},
      {1.3,0,2.55},{1.3,-0.728,2.55},{0.728,-1.3,2.55},{0,-1.3,2.55},
      {1.3,0,2.4},{1.3,-0.728,2.4},{0.728,-1.3,2.4},{0,-1.3,2.4},
    },
    { // Patch 25
      {0,-0.2,2.7},{-0.112,-0.2,2.7},{-0.2,-0.112,2.7},{-0.2,0,2.7},
      {0,-0.4,2.55},{-0.224,-0.4,2.55},{-0.4,-0.224,2.55},{-0.4,0,2.55},
      {0,-1.3,2.55},{-0.728,-1.3,2.55},{-1.3,-0.728,2.55},{-1.3,0,2.55},
      {0,-1.3,2.4},{-0.728,-1.3,2.4},{-1.3,-0.728,2.4},{-1.3,0,2.4},
    },
    { // Patch 26
      {-0.2,0,2.7},{-0.2,0.112,2.7},{-0.112,0.2,2.7},{0,0.2,2.7},
      {-0.4,0,2.55},{-0.4,0.224,2.55},{-0.224,0.4,2.55},{0,0.4,2.55},
      {-1.3,0,2.55},{-1.3,0.728,2.55},{-0.728,1.3,2.55},{0,1.3,2.55},
      {-1.3,0,2.4},{-1.3,0.728,2.4},{-0.728,1.3,2.4},{0,1.3,2.4},
    },
    { // Patch 27
      {0,0.2,2.7},{0.112,0.2,2.7},{0.2,0.112,2.7},{0.2,0,2.7},
      {0,0.4,2.55},{0.224,0.4,2.55},{0.4,0.224,2.55},{0.4,0,2.55},
      {0,1.3,2.55},{0.728,1.3,2.55},{1.3,0.728,2.55},{1.3,0,2.55},
      {0,1.3,2.4},{0.728,1.3,2.4},{1.3,0.728,2.4},{1.3,0,2.4},
    },
    { // Patch 28
      {0,0,0},{0,0,0},{0,0,0},{0,0,0},
      {1.425,0,0},{1.425,0.798,0},{0.798,1.425,0},{0,1.425,0},
      {1.5,0,0.075},{1.5,0.84,0.075},{0.84,1.5,0.075},{0,1.5,0.075},
      {1.5,0,0.15},{1.5,0.84,0.15},{0.84,1.5,0.15},{0,1.5,0.15},
    },
    { // Patch 29
      {0,0,0},{0,0,0},{0,0,0},{0,0,0},
      {0,1.425,0},{-0.798,1.425,0},{-1.425,0.798,0},{-1.425,0,0},
      {0,1.5,0.075},{-0.84,1.5,0.075},{-1.5,0.84,0.075},{-1.5,0,0.075},
      {0,1.5,0.15},{-0.84,1.5,0.15},{-1.5,0.84,0.15},{-1.5,0,0.15},
    },
    { // Patch 30
      {0,0,0},{0,0,0},{0,0,0},{0,0,0},
      {-1.425,0,0},{-1.425,-0.798,0},{-0.798,-1.425,0},{0,-1.425,0},
      {-1.5,0,0.075},{-1.5,-0.84,0.075},{-0.84,-1.5,0.075},{0,-1.5,0.075},
      {-1.5,0,0.15},{-1.5,-0.84,0.15},{-0.84,-1.5,0.15},{0,-1.5,0.15},
    },
    { // Patch 31
      {0,0,0},{0,0,0},{0,0,0},{0,0,0},
      {0,-1.425,0},{0.798,-1.425,0},{1.425,-0.798,0},{1.425,0,0},
      {0,-1.5,0.075},{0.84,-1.5,0.075},{1.5,-0.84,0.075},{1.5,0,0.075},
      {0,-1.5,0.15},{0.84,-1.5,0.15},{1.5,-0.84,0.15},{1.5,0,0.15},
    },
  };
  oPRCFile file("test.prc");
  PRCsurface *psn[NUMBER_OF_PATCHES];
  const int N_COLOURS = 32;
  RGBAColour colours[N_COLOURS];
  for(int i = 0; i < N_COLOURS; ++i)
  {
    colours[i%N_COLOURS].R = 0.0;
    colours[i%N_COLOURS].G = (i%N_COLOURS)/static_cast<double>(N_COLOURS);
    colours[i%N_COLOURS].B = 0.95;
    colours[i%N_COLOURS].A = 0.75;
  }
  for(int i = 0; i < NUMBER_OF_PATCHES; ++i)
  {
    psn[i] = new PRCsurface(&file,3,3,4,4,controlPoints[i],knotsU,knotsV,colours[i%N_COLOURS]);
    file.add(psn[i]);
  }

  const int NUMBER_OF_POINTS = 31;
  double points[NUMBER_OF_POINTS][3];
  for(int i = 0; i < NUMBER_OF_POINTS; ++i)
  {
    points[i][0] = 3.5*cos(3.0*i/NUMBER_OF_POINTS*2.0*M_PI);
    points[i][1] = 3.5*sin(3.0*i/NUMBER_OF_POINTS*2.0*M_PI);
    points[i][2] = 5.0*i/NUMBER_OF_POINTS-1.0;
  }
  double knots[3+NUMBER_OF_POINTS+1];
  knots[0] = 1;
  for(int i = 1; i < 3+NUMBER_OF_POINTS; ++i)
  {
    knots[i] = (i+2)/3; // integer division is intentional
  }
  knots[3+NUMBER_OF_POINTS] = (3+NUMBER_OF_POINTS+1)/3;

  RGBAColour white(1.0,1.0,1.0);
  PRCcurve pc(&file,3,NUMBER_OF_POINTS,points,knots,white);
  file.add(&pc);

  RGBAColour red(1.0,0.0,0.0);
  PRCline pl(&file,NUMBER_OF_POINTS,points,red);
  file.add(&pl);

  file.finish();

  for(int i = 0; i < NUMBER_OF_PATCHES; ++i)
    delete psn[i];

  return 0;
}
