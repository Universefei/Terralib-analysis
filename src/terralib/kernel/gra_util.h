#ifndef GRAPH_UTILITIES_H
#define GRAPH_UTILITIES_H

#include<place.h>
#include<graph.h>
#include<fstream>
#include<myrandom.h>
#include<string>
#include<iostream>

namespace br_stl {

/* During automatic creation of an undirected graph, a name must be
   generated for each vertex. The following auxiliary function
   converts the current number into a string object which is entered
   as identifier. */

// auxiliary function for generating strings out of numbers
std::string i2string(unsigned int i) {
    if(i==0) return std::string("0");
    char buf[] = "0000000000000000";
    char *pos = buf + sizeof(buf)-1;  // point to end
   
    do
      *--pos = i % 10 + '0';
    while(i /=10);
    return std::string(pos);
}

/* The function create_vertex_set() creates a number of vertices with
   random coordinates between 0 and maxX or maxY in a graph G
   according to its size (G.size()). */
   
template<class EdgeType>
void create_vertex_set(Graph<Place, EdgeType>& G,
                  int count, int maxX, int maxY) {
    Random xRandom(maxX),
           yRandom(maxY);

    // create vertices with random coordinates
    int i = -1;
    while(++i < count)
      G.insert(Place(xRandom(), yRandom(),i2string(i)));
}

template<class EdgeType>
void connectNeighbors(Graph<Place, EdgeType>& G) {
    for(size_t i = 0; i < G.size(); ++i) {
       Place iPlace = G[i].first;

       for(size_t j = i+1; j < G.size(); ++j) {
          Place jPlace = G[j].first;

          Place MidPoint((iPlace.X()+jPlace.X())/2,
                         (iPlace.Y()+jPlace.Y())/2);

         /* The following loop is not run-time optimized. A possible
            optimization could be to sort the places by their x
            coordinates so that only a small relevant range must be
            searched. The relevant range results from the fact that
            the places to be compared must lie inside a circle around
            the mid-point whose diameter is equal to the distance
            between the places i and j. */

          size_t k = 0;
          unsigned long int e2 = DistSquare(iPlace, MidPoint);

          while(k < G.size()) {      // not run-time optimized
             if(k != j && k != i &&
                DistSquare(G[k].first, MidPoint) < e2)
                    break;
             ++k;
          }

          if(k == G.size())  {// no nearer place found
             EdgeType dist = Distance(iPlace, jPlace);
             G.connectVertices(i, j, dist);
          }
       }
    }
}


// Only for undirected graphs!
template<class EdgeType>
void createTeXfile(const char * Filename,
                   Graph<Place, EdgeType>& G,
                   double ScalingFactor,
                   int xMax, int yMax) {
    assert(!G.isDirected());
    std::ofstream Output(Filename);

    if(!Output) {
        std::cerr << Filename
             << " cannot be opened!\n";
        exit(1);
    }

    Output  << "%% This is a generated file!\n"
         << "\\unitlength 1.00mm\n"
         << "\\begin{picture}("
         << xMax << ','
         << yMax << ")\n";

    for(size_t iv = 0; iv < G.size(); ++iv) {
       // Point
       Output << "\\put("
               << G[iv].first.X()*ScalingFactor
               << ','
               << G[iv].first.Y()*ScalingFactor
               << "){\\circle*{1.0}}\n";

       // node name
       Output << "\\put("
               << (1.0 + G[iv].first.X()*ScalingFactor)
               << ','
               << G[iv].first.Y()*ScalingFactor
               << "){\\makebox(0,0)[lb]{{\\tiny "
               << G[iv].first            // name
               << "}}}\n";

       /* All edges are drawn. In order to prevent them from appearing
          twice in the undirected graph, they are only drawn in the
          direction of the greater index. */

       typename Graph<Place,EdgeType>::Successor::const_iterator I =
         //       std::map< int, EdgeType >::const_iterator I =
                           G[iv].second.begin();

       while(I != G[iv].second.end()) {
          size_t n = (*I).first;
          if(n > iv) {             // otherwise, ignore
             double x1,x2,y1,y2,dx,dy;
             x1 =  G[iv].first.X()*ScalingFactor;
             y1 =  G[iv].first.Y()*ScalingFactor;
             x2 =  G[n].first.X()*ScalingFactor;
             y2 =  G[n].first.Y()*ScalingFactor;
             dx = x2-x1; dy = y2-y1;
             double dist = std::sqrt(dx*dx+dy*dy);
             int wdh = int(5*dist);
             dx = dx/wdh; dy = dy/wdh;
             Output << "\\multiput(" << x1 << "," << y1 << ")("
              << dx << "," << dy << "){" << wdh
                   << "}{\\circle*{0.1}}\n";
          }
          ++I;
       }
    }
    Output << "\\end{picture}\n";
}

/* This is similar to createTeXfile, but creates
   metapost-output which can with metapost directly
   be converted to postscript. The adaption of this routine
   was made by Andreas Scherer.
   Only for undirected graphs! */

template<class EdgeType>
void createMPfile(char * Filename,
                  Graph<Place, EdgeType>& G,
                  double ScalingFactor) {
    assert(!G.isDirected());
    std::ofstream Output(Filename);

    if(!Output) {
        std::cerr << Filename
             << " cannot be opened!\n";
        exit(1);
    }

    Output  << "%%%% createMPfile(): This is a generated file!\n"
         << "input boxes\n\n"
         << "beginfig(1);\nu=1mm; % unitlength\n"
         << "defaultscale := 0.8; % small numbers\n";

    for(register size_t iv = 0; iv < G.size(); ++iv) {
       // Point
       Output << " drawdot( "
               << G[iv].first.X()*ScalingFactor
               << "u, "
               << G[iv].first.Y()*ScalingFactor
               << "u)  withpen pencircle scaled 1mm;\n";

       // node name
       Output << " label.urt( \""
              << G[iv].first << "\", ( "
               << G[iv].first.X()*ScalingFactor
               << "u, "
               << G[iv].first.Y()*ScalingFactor
               << "u) ); " << std::endl;

       /* All edges are drawn. In order to prevent them from appearing
          twice in the undirected graph, they are only drawn in the
          direction of the greater index. */


       typename Graph<Place, EdgeType>::Successor::const_iterator I =
         //        std:: map<int, EdgeType>::const_iterator I =
                           G[iv].second.begin();

       while(I != G[iv].second.end()) {
          size_t n = (*I).first;
          if(n > iv) {             // otherwise, ignore
             Output << "  draw ( "
                    << G[iv].first.X()*ScalingFactor << "u, "
                    << G[iv].first.Y()*ScalingFactor 
                    << "u ) -- ( "
                    << G[n].first.X()*ScalingFactor << "u, "
                    << G[n].first.Y()*ScalingFactor
                    << "u );" << std::endl;
          }
          ++I;
       }
    }
    Output << "endfig;" << std::endl << "end." << std::endl;
}

/* This function (written by A. Scherer) writes a path
   within a graph as a separate metapost-file */

template<class EdgeType>
void writeMPpath(char * Filename,
                 Graph<Place, EdgeType>& G,
                 std::vector< int >& V,
                 double ScalingFactor,
                 int Start) {
    std::ofstream Output(Filename);

    if(!Output) {
        std::cerr << Filename
             << " cannot be opened!\n";
        exit(1);
    }

    Output  << "%%%% writeMPpath(): This is a generated file!\n"
            << "beginfig( 1 );\nu=1mm; % unitlength" << std::endl
            << "pickup pencircle scaled 2pt;\n"  // line width
            << "  draw ( "
            << G[ Start ].first.X() * ScalingFactor
         << "u, "
         << G[ Start ].first.Y() * ScalingFactor
         << "u)";

     while ( V[ Start ] >= 0 ) {
         int Successor = V[ Start ];
         Output << std::endl << "    -- ( "
             << G[ Successor ].first.X() * ScalingFactor
             << "u, "
             << G[ Successor ].first.Y() * ScalingFactor
             << " u)";
         Start = Successor;
     }
     Output << ";\n" << "endfig;\n" << "end." << std::endl;
}

} // namespace br_stl

#endif  // graphUtilities

