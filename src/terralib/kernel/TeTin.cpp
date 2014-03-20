/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright Â© 2001-2007 INPE and Tecgraf/PUC-Rio.

This code is part of the TerraLib library.
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

You should have received a copy of the GNU Lesser General Public
License along with this library.

The authors reassure the license terms regarding the warranties.
They specifically disclaim any warranties, including, but not limited to,
the implied warranties of merchantability and fitness for a particular purpose.
The library provided hereunder is on an "as is" basis, and the authors have no
obligation to provide maintenance, support, updates, enhancements, or modifications.
In no event shall INPE and Tecgraf / PUC-Rio be held liable to any party for direct,
indirect, special, incidental, or consequential damages arising out of the use
of this library and its documentation.
*************************************************************************************/

#include "TeAsciiFile.h"
#include "TeTin.h"
//#include "TeStdFile.h"
#include "TeException.h"
#include "TeGeometryAlgorithms.h"
#include "TeUtils.h"
#include "TeRTree.h"
//#include "geo.h"

#include <iostream>
#include <fstream>
#include <algorithm>

// TeTinTriangleSet

bool
TeTinTriangleSet::loadFromFile(std::string &fileName)
{
//	Open tin triangles file
	std::string trianglesFile = fileName;
	trianglesFile += ".tpl";

	std::ifstream inStream;
	inStream.open( trianglesFile.data(), std::ios::in | std::ios::binary );

	if (! inStream.good() )
		return false;

	long laux;
//#if defined(SPRWIN) || defined(LITTLEENDIAN) - File is saved in LITTLEENDIAN
	inStream.read((char*)&laux, sizeof(long));
	long numTriangles = laux;

//	Create and std::set tin triangles std::vector
	if ( this->size () )
		this->clear ();
	this->reserve (numTriangles);

//	Read file for triangles data load
	long lineid [3];
	for (long i = 0; i < numTriangles; i++)
	{
		for (long j = 0; j < 3; j++)
		{
			if (! inStream.good() )
				return false;
			inStream.read((char*)&laux, sizeof(long));
			lineid[j] = laux;
		}
		this->add ( TeTinTriangle (lineid[0],lineid[1],lineid[2]) );
	}
	return true;
}

bool
TeTinVertexSet::loadFromFile(std::string &fileName)
{
//	Open tin vertices file
	std::string verticesFile = fileName;
	verticesFile += ".tnd";

	std::ifstream inStream;
	inStream.open( verticesFile.data(), std::ios::in | std::ios::binary );

	if (! inStream.good() )
		return false;

	long laux;
//#if defined(SPRWIN) || defined(LITTLEENDIAN) - File is saved in LITTLEENDIAN
	inStream.read((char*)&laux, sizeof(long));
	long numVertices = laux;

//	Create and std::set tin vertices std::vector
	if ( this->size () )
		this->clear ();
	this->reserve (numVertices);

//	Read file for vertices data load
	double x, y;
	double value;
	short vertexType;
	for (long i = 0; i < numVertices; i++)
	{
		if (! inStream.good() )
			return false;
		inStream.read((char*)&x, sizeof(double));
		if (! inStream.good() )
			return false;
		inStream.read((char*)&y, sizeof(double));
		if (! inStream.good() )
			return false;
		inStream.read((char*)&value, sizeof(double));
		if (! inStream.good() )
			return false;
		inStream.read((char*)&vertexType, sizeof(short));
		this->add( TeTinVertex (TeCoord2D (x,y), value,
			(TeTinVertexType) vertexType, -1 ) );
	}
	return true;
}

bool
TeTinEdgeSet::loadFromFile(std::string &fileName)
{
//	Open tin edges file
	std::string edgesFile = fileName;
	edgesFile += ".tln";

	std::ifstream inStream;
	inStream.open( edgesFile.data(), std::ios::in | std::ios::binary );

	if (! inStream.good() )
		return false;

	long laux;
//#if defined(SPRWIN) || defined(LITTLEENDIAN) - File is saved in LITTLEENDIAN
	inStream.read((char*)&laux, sizeof(long));
	long numEdges = laux;

//	Create and std::set tin edges std::vector
	if ( this->size () )
		this->clear ();
	this->reserve (numEdges);

//	Read file for edges data load
	long from, to, left, right;
	for (long i = 0; i < numEdges; i++)
	{
		if (! inStream.good() )
			return false;
		inStream.read((char*)&from, sizeof(long));
		if (! inStream.good() )
			return false;
		inStream.read((char*)&to, sizeof(long));

		if (! inStream.good() )
			return false;
		inStream.read((char*)&left, sizeof(long));
		if (! inStream.good() )
			return false;
		inStream.read((char*)&right, sizeof(long));

		this->add( TeTinEdge (from, to, left, right, NORMALEDGE) );
	}
	return true;
}


// TeTin

bool
TeTin::loadFromFile(std::string &fileName)
{
	if ( triangleSet_.loadFromFile ( fileName ) )
	{
		if ( vertexSet_.loadFromFile ( fileName ) )
		{
			if ( edgeSet_.loadFromFile ( fileName ) )
				return true;
		}
	}
	return false;
}

bool
TeTin::getLines(TeLineSet &lineSet)
{
	std::vector<TeTinEdge>::iterator i = edgeSet_.begin();
	while ( i != edgeSet_.end() )
	{
		long from = (*i).from();
		long to = (*i).to();
		TeCoord2D ptf = vertexSet_[from].location();
		TeCoord2D ptt = vertexSet_[to].location();
		i++;

		TeLine2D line;
		line.add (ptf);
		line.add (ptt);
		lineSet.add(line);
	}
	return true;
}

long
TeTin::oneEdgeWithVertex(long v)
{
	long vsize = vertexSet_.size();
	long edge;
	if (v < vsize)
	{
		TeTinVertex vv = vertexSet_[v];
		edge = vv.oneEdge();
	}
	else
		return -1;

	// Test to make sure there is no wrong index
	if ( edge != -1 )
		if ( ( edgeSet_[edge].to() == v ) ||
			 ( edgeSet_[edge].from() == v ) )
		return edge;

	//Fail safe code, must return from previous code
	static long	oldedge = 0;	// Set one edge only once

	long j = 0;
	while (j < 2)
	{
		unsigned long i;
		for (i = oldedge; i < edgeSet_.size(); i++)
		{
			long from = edgeSet_[i].from();
			long to   = edgeSet_[i].to();
			if ( from == -1 || to == -1 )
				continue;
			if (from == v || to == v)
			{
				oldedge = i;
				return i;
			}
		}
		oldedge = 0;
		j++;
	}
	return -1;
}

long
TeTin::edgeOppVertex(TeTinTriangle& t, long v)
{
	long edges[3];
	t.edges(edges[0],edges[1],edges[2]);

	for (long i = 0; i < 3; i++)
	{
		if ( edgeSet_[edges[i]].from() != v &&
		     edgeSet_[edges[i]].to()   != v )
			return edges[i];
	}
	return -1;
}

long
TeTin::vertexOppEdge(long t, long e)
{
	long edges[3];
	triangleSet_[t].edges(edges[0],edges[1],edges[2]);

	long vef(-1), vet(-1);
	unsigned long i;
	for (i = 0; i < 3; i++)
	{
		if ( edges[i] == e )
		{
			vef = edgeSet_[edges[i]].from();
			vet = edgeSet_[edges[i]].to();
			break;
		}
	}
	if ( vef == -1 )
		return -1;

	for (i = 0; i < 3; i++)
	{
		if ( edges[i] != e )
		{
			if ( vef == edgeSet_[edges[i]].from() ||
			     vet == edgeSet_[edges[i]].from() )
				return edgeSet_[edges[i]].to();
			if ( vef == edgeSet_[edges[i]].to() ||
			     vet == edgeSet_[edges[i]].to() )
				return edgeSet_[edges[i]].from();
			return -1;
		}
	}
	return -1;
}

long
TeTin::edgeWithVertexDiffFromEdge(long t, long v, long e)
{
	long edges[3];
	triangleSet_[t].edges(edges[0],edges[1],edges[2]);

	for (long i = 0; i < 3; i++)
	{
		if (( edges[i] == e ) || ( edges[i] == -1))
			continue;
		if ( edgeSet_[edges[i]].from() == v ||
		     edgeSet_[edges[i]].to()   == v )
			return edges[i];
	}
	return -1;
}

bool
TeTin::vertexOppEdges(long v, std::list<long> &edges)
{
//	Find one line that contains vertex
	long a = oneEdgeWithVertex(v);
	if (a == -1)
		return false;

//	1. Defina td como sendo o triângulo que está à direita da aresta a e
	long td = edgeSet_[a].right(); // te como sendo o triângulo que está
	long te = edgeSet_[a].left();  // à esquerda de a,

	long ai = a; // 2. Defina ai como sendo aresta a 
	long ti = td;//    e ti como sendo o triângulo td,

	long ao;
 
	if ( ti != -1 ) // 3. Se o triângulo ti não for nulo,
	{				//    insira aresta ao de ti que não é diretamente
		ao = edgeOppVertex ( ti, v );// conectado a v no conjunto A,
		if (ao == -1)
			return false;
		edges.push_back (ao);
	}

	while (ti != te)// 4. Enquanto ti for diferente do triângulo te,
	{
		if (ti == -1)//	4.1. Se o triângulo ti é nulo (esta' na
		{			 //  borda da triangulação) faça:
			ti = te; //	 4.1.1. Defina ti como sendo o triângulo te,
			te = -1; //  4.1.2. Defina te como sendo nulo,
			ai = a;  //  4.1.3. Defina ai como sendo aresta a,
			if ( ti != -1 ) // 4.1.4. Se o triângulo ti não for nulo,
			{				// insira aresta ao de ti que não é diretamente
				ao = edgeOppVertex ( ti, v );// conectado a v
				if (ao == -1)
					return false;
				edges.push_back (ao); // a v no conjunto A,
			}
			continue;//	 4.1.5. Retorne a 4.
		}

		long aaux = edgeWithVertexDiffFromEdge ( ti, v, ai );// 4.2. Defina a
		if (aaux == -1)	 // aresta aaux do triângulo ti que conecta o
			return false;// vértice v e é diferente de ai,

		long taux;
		if (edgeSet_[aaux].right() == ti) // 4.3. Defina taux como sendo  
			taux = edgeSet_[aaux].left();//  o triângulo que compartilha
		else if (edgeSet_[aaux].left() == ti)// a aresta aaux com ti,
			taux = edgeSet_[aaux].right();
		else
			return false;

		ti = taux;// 4.4. Defina ti como sendo o triângulo taux e ai 
		ai = aaux;//	  como sendo aresta aaux,

		if ( ti != -1 )// 4.5. Se o triângulo ti não for nulo, insira
		{			   //      aresta ao de ti que não é diretamente
			ao = edgeOppVertex ( ti, v ); // conectado a v
			if (ao == -1)							// no conjunto A,
				return false;
			edges.push_back (ao);
		}
	} // 4.6. Retorne a 4.
	return true;
}

bool
TeTin::vertexEdges(long v, std::list<long> &edges)
{
//	Find one line that contains vertex
	long a = oneEdgeWithVertex(v);
	if (a == -1)
		return false;

//	1. Defina td como sendo o triângulo que está à direita da aresta a e
	long td = edgeSet_[a].right(); // te como sendo o triângulo que está
	long te = edgeSet_[a].left();  // à esquerda de a,

	long ai = a; // 2. Defina ai como sendo aresta a 
	long ti = td;//    e ti como sendo o triângulo td,

	edges.push_back (ai);//	3. Insira a aresta ai no conjunto A,

	while (ti != te)//	4. Enquanto ti for diferente do triângulo te,
	{
		if (ti == -1)
		{// 4.1. Se o triângulo ti é nulo (esta' na borda da triangulação) faça:
			ti = te;// 4.1.1. Defina ti como sendo o triângulo te,
			te = -1;// 4.1.2. Defina te como sendo nulo,
			ai = a;// 4.1.3. Defina ai como sendo aresta a,
			continue;// 4.1.4. Retorne a 4.
		}

		long aaux = edgeWithVertexDiffFromEdge ( ti, v, ai );// 4.2. Defina a
		if (aaux == -1)	 // aresta aaux do triângulo ti que conecta o
			return false;// vértice v e é diferente de ai,

		long taux;
		if (edgeSet_[aaux].right() == ti) // 4.3. Defina taux como sendo  
			taux = edgeSet_[aaux].left();//  o triângulo que compartilha
		else if (edgeSet_[aaux].left() == ti)// a aresta aaux com ti,
			taux = edgeSet_[aaux].right();
		else
			return false;

		ti = taux;// 4.4. Defina ti como sendo o triângulo taux e ai 
		ai = aaux;//	  como sendo aresta aaux,

		edges.push_back (ai);// 4.5. Se o triângulo ti não for nulo, insira
	} // 4.6. Retorne a 4.
	return true;
}

bool
TeTin::vertexOppVertices(long v, std::list<long> &vertices)
{
//	Find one line that contains vertex
	long a = oneEdgeWithVertex(v);
	if (a == -1)
		return false;

//	1. Defina td como sendo o triângulo que está à direita da aresta a e
	long td = edgeSet_[a].right(); // te como sendo o triângulo que está
	long te = edgeSet_[a].left();  // à esquerda de a,

	long ai = a; // 2. Defina ai como sendo aresta a 
	long ti = td;//    e ti como sendo o triângulo td,

	long vi = edgeSet_[a].from();// 3. Insira o vértice diferente de v
	if (vi == v)				// conectado à aresta ai no conjunto V,
		vi = edgeSet_[a].to();
	vertices.push_back (vi);

	while (ti != te)// 4. Enquanto ti for diferente do triângulo te,
	{
		if (ti == -1)// 4.1. Se o triângulo ti é nulo (esta' na borda 
		{			 // da triangulação) faça:
			ti = te; //	4.1.1. Defina ti como sendo o triângulo te,
			te = -1; //	4.1.2. Defina te como sendo nulo,
			ai = a;  //	4.1.3. Defina ai como sendo aresta a,
			continue;//	4.1.4. Retorne a 4.
		}

		long aaux = edgeWithVertexDiffFromEdge ( ti, v, ai );// 4.2. Defina a
		if (aaux == -1)	 // aresta aaux do triângulo ti que conecta o
			return false;// vértice v e é diferente de ai,

		long taux;
		if (edgeSet_[aaux].right() == ti) // 4.3. Defina taux como sendo  
			taux = edgeSet_[aaux].left();//  o triângulo que compartilha
		else if (edgeSet_[aaux].left() == ti)// a aresta aaux com ti,
			taux = edgeSet_[aaux].right();
		else
			return false;

		ti = taux;// 4.4. Defina ti como sendo o triângulo taux e ai 
		ai = aaux;//	  como sendo aresta aaux,

		vi = edgeSet_[ai].from();//	4.5. Insira o vértice diferente
		if (vi == v)	// de v conectado à aresta ai no conjunto V,
			vi = edgeSet_[ai].to();
		vertices.push_back (vi);
	}//	4.6. Retorne a 4.
	return true;
}

bool
TeTin::vertexTriangles(long v, std::list<long> &triangles)
{
//	Find one line that contains vertex
	long a = oneEdgeWithVertex(v);
	if (a == -1)
		return false;

//	1. Defina td como sendo o triângulo que está à direita da aresta a e
	long td = edgeSet_[a].right(); // te como sendo o triângulo que está
	long te = edgeSet_[a].left();  // à esquerda de a,

	long ai = a; // 2. Defina ai como sendo aresta a 
	long ti = td;//    e ti como sendo o triângulo td,
 
	if ( ti != -1 ) // 3. Se o triângulo ti não for nulo,
	{				//    insira ti no conjunto T,
		triangles.push_back (ti);
	}

	while (ti != te)// 4. Enquanto ti for diferente do triângulo te,
	{
		if (ti == -1)//	4.1. Se o triângulo ti é nulo (esta' na
		{			 //  borda da triangulação) faça:
			ti = te; //	 4.1.1. Defina ti como sendo o triângulo te,
			te = -1; //  4.1.2. Defina te como sendo nulo,
			ai = a;  //  4.1.3. Defina ai como sendo aresta a,
			if ( ti != -1 ) // 4.1.4. Se o triângulo ti não for nulo,
			{				// insira ti no conjunto T,
				triangles.push_back (ti);
			}
			continue;//	 4.1.5. Retorne a 4.
		}

		long aaux = edgeWithVertexDiffFromEdge ( ti, v, ai );// 4.2. Defina a
		if (aaux == -1)	 // aresta aaux do triângulo ti que conecta o
			return false;// vértice v e é diferente de ai,

		long taux;
		if (edgeSet_[aaux].right() == ti) // 4.3. Defina taux como sendo  
			taux = edgeSet_[aaux].left();//  o triângulo que compartilha
		else if (edgeSet_[aaux].left() == ti)// a aresta aaux com ti,
			taux = edgeSet_[aaux].right();
		else
			return false;

		ti = taux;// 4.4. Defina ti como sendo o triângulo taux e ai 
		ai = aaux;//	  como sendo aresta aaux,

		if ( ti != -1 )// 4.5. Se o triângulo ti não for nulo, insira
		{			   //      aresta ao de ti que não é diretamente
			triangles.push_back (ti);
		}
	} // 4.6. Retorne a 4.
	return true;
}

bool
TeTin::vertexOppEdges(long v, std::list<long> &triset, std::list<long> &edges)
{
	std::list<long>::iterator tii;
	for (tii = triset.begin(); tii != triset.end(); tii++)
	{
		long ti = *tii;
		long nedge = this->edgeOppVertex(ti, v);
		edges.push_back(nedge);
	}
	return true;
}

long
TeTin::oneTriangleWithVertex(long v)
{
//	Find one line that contains vertex
	long a = oneEdgeWithVertex(v);
	if (a == -1)
		return -1;

	long td = edgeSet_[a].right();
	if ( td == -1 )
		return edgeSet_[a].left();
	else
		return td;
}

bool
TeTin::triangleContainsPoint(TeTinTriangle& t, TeCoord2D& pt) 
{
	double	totalArea, triangleArea;
	TeCoord2D	vert[3];

	this->trianglePoints(t, vert[0], vert[1], vert[2]);

//	Calculate the base triangle area
	triangleArea = fabs(( (vert[1].x() - vert[0].x()) *
		              (vert[2].y() - vert[0].y()) ) -
	 	            ( (vert[2].x() - vert[0].x()) *
	 	              (vert[1].y() - vert[0].y()) ));
	triangleArea *= 1.00001;
	totalArea = fabs(( (vert[0].x() - pt.x()) * (vert[1].y() - pt.y()) ) -
		         ( (vert[1].x() - pt.x()) * (vert[0].y() - pt.y()) ));
	if (totalArea > triangleArea)
		return false;

	totalArea += fabs(( (vert[1].x() - pt.x()) * (vert[2].y() - pt.y()) ) -
		          ( (vert[2].x() - pt.x()) * (vert[1].y() - pt.y()) ));
	if (totalArea > triangleArea) 
		return false;

	totalArea += fabs(( (vert[0].x() - pt.x()) * (vert[2].y() - pt.y()) ) -
		          ( (vert[2].x() - pt.x()) * (vert[0].y() - pt.y()) ));
	if (totalArea > triangleArea) 
		return false;

	return true;
}

long
TeTin::triangleAt (TeCoord2D& pt)
{
//Seja v um vértice qualquer da triangulação T e p o ponto para o qual se deseja saber qual triângulo t
//	o contém.
	long vi1;

	TeTinEdgeSet::reverse_iterator i = edgeSet_.rbegin();
	while ( i != edgeSet_.rend() )
	{
		vi1 = (*i++).from();
		if (vi1 >= 0 ) // -1L)
			break;
	}
	if ( i == edgeSet_.rend() )
		return -1;

//1. Defina o conjunto T={t1,..., tm} com os m triângulos que compartilham o vértice vi,
	std::list<long> triset;
	this->vertexTriangles(vi1, triset);

	TeCoord2D pf = vertexSet_[vi1].location();
	if ( pf == pt )
//2. Se vi é igual a p, o triângulo que contém p é um dos triângulos que compartilham vi.
// Escolha qualquer um dos triângulos e termine o algoritmo.
		return ( *(triset.begin()) );

//3. Defina A={a1,...,an} com as n arestas opostas a vi,
	std::list<long> aedges;
	this->vertexOppEdges(vi1, triset, aedges);

//4. Defina V={v1,...,vm} com os m vértices dos triângulos que contém o vértice vi,
	std::list <long> vvertex;
	this->vertexOppVertices(vi1, vvertex);

//5. Insira o vértice v no conjunto VI de vértices intersectados,
	std::set <long> viset;
	viset.insert(vi1);

//6. Defina o vértice auxiliar vaux como sendo vi,
	long vaux = vi1;
//   e aresta aaux como sendo inválida
	long aaux = -1;
	std::vector<long> auxset(3);
	for (;;)
	{
//7. Para cada triângulo ti de T, faça:
		std::list<long>::iterator tii;
		for (tii = triset.begin(); tii != triset.end(); tii++)
		{
			long ti = *tii;
//7.1. Se o triângulo ti contém o ponto pn, termine o algoritmo.
			if ( this->triangleContainsPoint( ti, pt ) )
				return ti;
		}
//8.	Para cada vértice vi de V, faça:
		std::list<long>::iterator vii;
		for (vii = vvertex.begin(); vii != vvertex.end(); vii++)
		{
			long vi = *vii;
			TeCoord2D ptvi = vertexSet_[vi].location();
			if ( ptvi == pt )
//8.1. Se vi é igual a p, o triângulo que contém p é um dos triângulos que compartilham vi.
			{
				for (tii = triset.begin(); tii != triset.end(); tii++)
				{
					long ti = *tii;
					long v0, v1, v2;
					this->triangleVertices(ti, v0, v1, v2);
					if ( v0 == vi || v1 == vi || v2 == vi)
						return ti;
				}
				return -1;
			}
		}

		bool lastvertex = true;
//9. Para cada vértice vi de V, faça:
		for ( vii = vvertex.begin(); vii != vvertex.end(); vii++)
		{
			lastvertex = false;

			long vi = *vii;
//9.1.	Se vi está sobre r, faça:
			TeCoord2D ptvi = vertexSet_[vi].location();
			TeCoord2D pinter;
			double tolerance = TePrecision::instance().precision();
			if ( TePerpendicularDistance(pf, pt, ptvi, pinter) < TePrecision::instance().precision() )
			{
				TeBox box1;
				updateBox (box1, pf);
				updateBox (box1, pt);
				//extra cases
				TeLine2D linter;
				linter.add(pf);
				linter.add(pt);
				if (TeWithin (pinter, box1) || (TeWithin(pinter,linter)))
				{
//9.1.1.	Redefina o conjunto A={a1,..., an} com as n arestas opostas a vi, diferentes de aaux,
					aedges.clear();
					this->vertexOppEdges(vi, aedges);
//9.1.2.	Defina o conjunto Aaux={a1,..., am} com as m arestas compartilhadas por vaux,
					std::list <long> vauxedges;
					this->vertexEdges(vaux, vauxedges);
//9.1.3.	Exclua do conjunto A todas as arestas que pertencem ao conjunto Aaux,
					for (std::list<long>::iterator vai = vauxedges.begin(); vai != vauxedges.end(); vai++)
						aedges.remove ( *vai );
//9.1.4.	Exclua do conjunto A a aresta aaux,
					aedges.remove ( aaux ); // better perfomance than remove algorithm
//9.1.5.	Redefina o conjunto V={v1,..., vm} com os m vÃ©rtices dos triÃ¢ngulos que contÃ©m o vÃ©rtice vi,
//			diferentes de vaux,
					vvertex.clear();
					this->vertexOppVertices(vi, vvertex);
//9.1.6.	Exclua do conjunto V todos os vÃ©rtices que pertencem ao conjunto VI,
					for (std::set<long>::iterator vvi = viset.begin(); vvi != viset.end(); vvi++)
						vvertex.remove ( *vvi );
//9.1.7.	Redefina o conjunto T={t1, ..., tk } com os k triÃ¢ngulos que contÃ©m o vÃ©rtice vi,
					triset.clear();
					this->vertexTriangles(vi, triset);
//9.1.8.	Redefina o vÃ©rtice auxiliar vaux como sendo vi,
					vaux = vi;
//9.1.8.	Insira o vÃ©rtice vi no conjunto VI de vÃ©rtices intersectados,
					viset.insert(vi);
//9.1.8.	Retorne a 7,
					//pf = vertexSet_[vi].location();
					break;
				}
			}

			lastvertex = true;
		}
		if (lastvertex == false)//if ( vii != vvertex.end() )
			continue; // do for (;;)

//10.	Para cada aresta ai de A, faÃ§a:
		std::list<long>::iterator aii;
		for (aii = aedges.begin(); aii != aedges.end(); aii++)
		{
			long ai = *aii;
//10.1.	Se o ponto p estÃ¡ sobre ai, faÃ§a:
			TeCoord2D pfr = vertexSet_[edgeSet_[ai].from()].location();
			TeCoord2D pto = vertexSet_[edgeSet_[ai].to()].location();
			TeCoord2D pinter;
			if ( TePerpendicularDistance(pfr, pto, pt, pinter) < TePrecision::instance().precision() )
			{
				TeBox box1;
				updateBox (box1, pfr);
				updateBox (box1, pto);
				if (TeWithin(pinter, box1) )
				{
//10.1.1. Escolha qualquer um dos triÃ¢ngulos que compartilham ai.
					long ti = edgeSet_[ai].left();
//10.1.2.	Termine o algoritmo.
					if ( (ti != -1) && (this->triangleContainsPoint(ti, pt)) )
						return ( ti );
					else
					{
						ti = edgeSet_[ai].right();
						if ( (ti != -1) && (this->triangleContainsPoint(ti, pt)) )
							return ( ti );
						return -1;
					}
				}
			}
		}

//11.	Para cada aresta ai de A, faÃ§a:
		for (aii = aedges.begin(); aii != aedges.end(); aii++)
		{
			long ai = *aii;
			long vt = edgeSet_[ai].to();
			long vf = edgeSet_[ai].from();
			TeCoord2D pi;
//11.1.	Se ai intersecta r, faÃ§a:
			if (TeSegmentsIntersectPoint(vertexSet_[vt].location(), vertexSet_[vf].location(), pf, pt, pi) )
			{
//11.1.2.	Defina os triÃ¢ngulos t1 e t2 que compartilham a aresta ai.
//11.1.3.	Se t2 estÃ¡ contido no conjunto T , entÃ£o faÃ§a taux=t1,
				long taux = edgeSet_[ai].left();
				if (taux == -1)
					return -1;
				std::list<long>::iterator pos1 = find (triset.begin(), triset.end(), taux);
				if ( pos1 != triset.end() )
//11.1.4.	SenÃ£o, faÃ§a taux=t2,
					taux = edgeSet_[ai].right();
				if (taux == -1)
					return -1;
//11.1.5.	Se o triÃ¢ngulo taux contÃ©m o ponto pn, termine o algoritmo.
				if ( this->triangleContainsPoint( taux, pt ) )
					return taux;
//11.1.6.	Redefina o conjunto A={a1, a2} com as arestas do triÃ¢ngulo taux diferentes de ai,
				triangleSet_[taux].edges ( auxset[0], auxset[1], auxset[2] );
				aedges.clear();
				aedges.insert( aedges.begin(), auxset.begin(), auxset.end() );
				aedges.remove( ai );
//11.1.7.	Redefina o conjunto V={v1}. O vÃ©rtice v1 Ã© o vÃ©rtice do triÃ¢ngulo taux que nÃ£o estÃ¡
//			em nenhuma extremidade da aresta ai,
				long vaux1 = edgeSet_[ai].from();
				long vaux2 = edgeSet_[ai].to();
				this->triangleVertices ( taux, auxset[0], auxset[1], auxset[2]);
				vvertex.clear();
				vvertex.insert ( vvertex.begin(), auxset.begin(), auxset.end() );
				vvertex.remove ( vaux1 );
				vvertex.remove ( vaux2 );

//11.1.8.	Redefina o conjunto T={taux},
				triset.clear();
				triset.push_back ( taux );
//11.1.9.	Redefina aaux como sendo ai,
				aaux = ai;
//11.1.10.	Retorne a 8;
				break;
			}
		}
//12.	Se nÃ£o hÃ¡ mais arestas em A, entÃ£o:
		//if ( aii == aedges.end() )
		//{
//12.1.	Para cada triÃ¢ngulo ti de T, faÃ§a:
//			TeTinTriangleIterator tii;
//			for ( tii = triangleBegin(); tii != triangleEnd(); tii++)
			long ti;
			long tEnd = triangleSet_.size();
			for ( ti = 0; ti < tEnd; ti++)
			{
//				TeTinTriangle ti = *tii;
	//12.1.1.	Se o triÃ¢ngulo ti contÃ©m o ponto pn, termine o algoritmo.
				if ( this->triangleContainsPoint( ti, pt ) )
					return ti;
			}
			if (ti == tEnd )
				return -1;
		//}
	} // Do for (;;)
	return -1;
}

long
TeTin::triangleAt(TeBox boxPt)
{
	TeCoord2D pt;
	pt.setXY(boxPt.x1(), boxPt.y1());

	std::vector<long> vecTriang;
	indexTriang_->search(boxPt, vecTriang);

	std::vector<long>::iterator tii = vecTriang.begin();
	while (tii != vecTriang.end())
	{
		long ti = *tii;
		if ( this->triangleContainsPoint( ti, pt ) )
			return ti;

		tii++;
	}

	return -1;
}

bool
TeTin::insertPoint(double x, double y, double value)
{
	double tol;
	if (minedgesize_ == 0)
		tol = TePrecision::instance().precision();
	else
	{
		tol = minedgesize_;
		//TePrecision::instance().setPrecision(tol);
	}

//	Get Point to be inserted from std::vector
	TeCoord2D	pt(x,y);
	if ( ! TeWithin (pt, vertexSet_.box()) )
		return false;

//	Find the triangle that owns the point pt	
	//long t = triangleAt ( pt );
	long t = triangleAt(TeBox(pt, pt));

	if (t == -1)
		return false;

	vertexSet_.add( TeTinVertex (pt, value, verttype_, -1 ) );
	long v = vertexSet_.size()-1;

	double ptx = vertexSet_[v].location().x();
	double pty = vertexSet_[v].location().y();

	TeCoord2D vert[3];
	trianglePoints(t, vert[0], vert[1], vert[2]);

	//long vertex1[3];
	//this->triangleVertices(t, vertex1[0], vertex1[1], vertex1[2]);

//	Verify if point is too close to one triangle vertex
	long j;
	for ( j = 0; j < 3; j++)
	{
		//if (pt == vert[j]) // Using precision to compare
		//	return false;

		if ((fabs(ptx - vert[j].x()) < tol) && (fabs(pty - vert[j].y()) < tol))
		{
			long vert1[3];
			this->triangleVertices(t, vert1[0], vert1[1], vert1[2]);
			this->vertexExchange(vert1[j], v);
			vertexSet_[vert1[j]].setType(DELETEDVERTEX);

			return false;
		}
	}

	long neighids[3];
	this->triangle3Neighbors(t, neighids[0],neighids[1],neighids[2]);
	long eds[3];
	triangleSet_[t].edges( eds[0], eds[1], eds[2] );

//	Test if the point is on an edge
	long nedge = -1;
	double dmin = TeMAXFLOAT;
	TeCoord2D pmin;
	for (j = 0; j < 3; j++)
	{
		if ( TeWithinOrTouches (pt, vert[j], vert[(j+1)%3]) == false )
			continue;

		TeCoord2D paux;
		double daux = TePerpendicularDistance(vert[j], vert[(j+1)%3], pt, paux);
		//double daux = this->pointToSegmentDistance(&vert[j], &vert[(j+1)%3], &pt, &paux);
		//if ((daux < TePrecision::instance().precision()) && (daux < dmin))
		//{//			On edge j    --> if ((daux < tol) && (daux < dmin))
		//	nedge = j;
		//	dmin = daux;
		//	pmin = paux;
		//}
		
		if ((daux < tol) && (daux < dmin))
		{//			On edge j
			nedge = j;
			dmin = daux;
			pmin = paux;
		}
	}

	//vertexSet_.add( TeTinVertex (pt, value, NORMALVERTEX, -1 ) );
	//vertexSet_.add( TeTinVertex (pt, value, type_, -1 ) );
	//long v = vertexSet_.size()-1;

	std::set<long> trianglesToTest;
	if (nedge == -1)	// Not on an edge
	{
		this->twoNewTriangles(t, v, trianglesToTest);	//If not, split triang
	}
	else
	{
		vertexSet_[v].location().setXY(pmin.x(), pmin.y());
		//ptx = pmin.x();
		//pty = pmin.y();

		for (j = 0; j < 3; j++)
		{	// Check if neighbor triangle opposite vertex is too close
			if ( neighids[j] == -1 )
				continue;
			//long oppVertex = edgeOppVertex(neighids[j], eds[j]);
			long oppVertex = this->oppositeVertex(neighids[j], eds[j]);
			long vsize = vertexSet_.size();
			if ( oppVertex < vsize )
			{
				TeCoord2D ptaux = vertexSet_[oppVertex].location();
				//if ( ptaux == pt )
				//{
				//	return false;
				//}
				//long vert1[3];
				//this->triangleVertices(t, vert1[0], vert1[1], vert1[2]);

				//if ((fabs(vertexSet_[vert1[j]].location().x() - ptaux.x()) < minedgesize_) && (fabs(vertexSet_[vert1[j]].location().y() - ptaux.y()) < minedgesize_))
				if ((fabs(ptx - ptaux.x()) < tol) && (fabs(pty - ptaux.y()) < tol))
				{
				//	// If necessary add code to change oppVertex type and value
					long vert1[3];
					this->triangleVertices(t, vert1[0], vert1[1], vert1[2]);				
					this->vertexExchange(vert1[j], v);
					vertexSet_[vert1[j]].setType(DELETEDVERTEX);

					return false;
				}
			}
		}

//		Duplicate triangle and its neighbor on the same edge
		long an0 = this->duplicateTriangle( t, nedge, v, trianglesToTest);
		if (neighids[nedge] != -1)
		{
			long tv = neighids[nedge];
			this->dupNeighTriangle( tv, an0, v, trianglesToTest);
		}
	}
	std::set<long> trianglesToTest1;
	trianglesToTest1 = trianglesToTest;

	this->testDelaunayForVertex(v, trianglesToTest);

	return true;
}

bool
TeTin::triangle3DPoints(TeTinTriangle& t, TeCoord2D& pt0, TeCoord2D& pt1,
					TeCoord2D& pt2, double& val0, double& val1, double& val2)
{
	long vertex0, vertex1, vertex2;
	this->triangleVertices(t, vertex0, vertex1, vertex2);

	pt0 = vertexSet_[vertex0].location();
	pt1 = vertexSet_[vertex1].location();
	pt2 = vertexSet_[vertex2].location();

	val0 = vertexSet_[vertex0].value();
	val1 = vertexSet_[vertex1].value();
	val2 = vertexSet_[vertex2].value();

	return true;
}

bool
TeTin::trianglePoints(TeTinTriangle& t, TeCoord2D& pt0, TeCoord2D& pt1, TeCoord2D& pt2)
{
	long vertex0, vertex1, vertex2;
	this->triangleVertices(t, vertex0, vertex1, vertex2);

	pt0 = vertexSet_[vertex0].location();
	pt1 = vertexSet_[vertex1].location();
	pt2 = vertexSet_[vertex2].location();

	return true;
}

bool
TeTin::triangleVertices(TeTinTriangle& t, long& vertex0, long& vertex1, long& vertex2)
{
	long edge0, edge1, edge2;
	t.edges(edge0,edge1,edge2);

	if (edgeSet_[edge0].to() == edgeSet_[edge1].to())
	{
		vertex0 = edgeSet_[edge0].from();
		vertex1 = edgeSet_[edge0].to();
		vertex2 = edgeSet_[edge1].from();
	}
	else if (edgeSet_[edge0].to() == edgeSet_[edge1].from())
	{
		vertex0 = edgeSet_[edge0].from();
		vertex1 = edgeSet_[edge0].to();
		vertex2 = edgeSet_[edge1].to();
	}
	else if (edgeSet_[edge0].from() == edgeSet_[edge1].from())
	{
		vertex0 = edgeSet_[edge0].to();
		vertex1 = edgeSet_[edge0].from();
		vertex2 = edgeSet_[edge1].to();
	}
	else if (edgeSet_[edge0].from() == edgeSet_[edge1].to())
	{
		vertex0 = edgeSet_[edge0].to();
		vertex1 = edgeSet_[edge0].from();
		vertex2 = edgeSet_[edge1].from();
	}
	else
		return false;

	return true;
}

bool
TeTin::triangle3Neighbors(long t, long& neighbor0, long& neighbor1, long& neighbor2)
{
	long edge0, edge1, edge2;
	triangleSet_[t].edges(edge0,edge1,edge2);

	if (edgeSet_[edge0].left() == t)
		neighbor0 = edgeSet_[edge0].right();
	else
		neighbor0 = edgeSet_[edge0].left();

	if (edgeSet_[edge1].left() == t)
		neighbor1 = edgeSet_[edge1].right();
	else
		neighbor1 = edgeSet_[edge1].left();

	if (edgeSet_[edge2].left() == t)
		neighbor2 = edgeSet_[edge2].right();
	else
		neighbor2 = edgeSet_[edge2].left();

	return true;
}

long
TeTin::triangleVertexAt(long t, long vertex)
{
	long vertices[3];
	this->triangleVertices ( t, vertices[0],vertices[1],vertices[2] );

	return vertices[vertex];
}

bool
TeTin::twoNewTriangles(long t, long v, std::set<long>& triangles)
{
	TeCoord2D pt0, pt1, pt2;

//1. Crie o vÃ©rtice vn com os dados da amostra s,
//	Sint4 vn = nodeId;

//2. Crie 2 novos triÃ¢ngulos t1 e t2,
	long t1 = triangleSet_.size();
	long t2 = t1+1;

//3. Crie a nova aresta an0 que conecta os vÃ©rtices vn e v0 e
//   tem t e t1 como os triÃ¢ngulos que compartilham a aresta.
	long v0 = this->triangleVertexAt(t,0); //v0 e' o vertice 0 de t
	edgeSet_.add( TeTinEdge (v, v0, t, t1, NORMALEDGE) );
	long an0 = edgeSet_.size()-1;
	vertexSet_[v].setOneEdge(an0);

//4. Crie a nova aresta an1 que conecta os vÃ©rtices vn e v1 e
// 	tem t1 e t2 como os triÃ¢ngulos que compartilham a aresta.
	long v1 = this->triangleVertexAt(t,1); //v1 e' o vertice 1 de t
	edgeSet_.add( TeTinEdge (v, v1, t1, t2, NORMALEDGE) );
	long an1 = edgeSet_.size()-1;

//5. Crie a nova aresta an2 que conecta os vÃ©rtices vn e v2 e
//	tem t2 e t como os triÃ¢ngulos que compartilham a aresta.
	long v2 = this->triangleVertexAt(t,2); //v2 e' o vertice 2 de t
	edgeSet_.add( TeTinEdge (v, v2, t2, t, NORMALEDGE) );
	long an2 = edgeSet_.size()-1;

//6. Troque o triÃ¢ngulo t por t1 na aresta a0,
	long a0 = triangleSet_[t].edgeAt(0); // retorna aresta 0 de t
	edgeSet_[a0].exchangeTriangle(t,t1);

//7. Defina as arestas de t1 como sendo an0, a0 e an1,
	triangleSet_.add ( TeTinTriangle (an0, a0, an1) );

	long indexT = triangleSet_.size() - 1;
	this->trianglePoints(indexT, pt0, pt1, pt2);

	TeBox boxT;
	updateBox(boxT, pt0);
	updateBox(boxT, pt1);
	updateBox(boxT, pt2);

	triangleSet_[indexT].setBox(boxT);
	indexTriang_->insert(boxT, indexT);

//8. Troque o triÃ¢ngulo t por t2 na aresta a1,
	long a1 = triangleSet_[t].edgeAt(1); // retorna aresta 1 de t
	edgeSet_[a1].exchangeTriangle(t, t2);

//9. Defina as arestas de t2 como sendo an1, a1 e an2,
	triangleSet_.add ( TeTinTriangle (an1, a1, an2) );

	indexT = triangleSet_.size() - 1;
	this->trianglePoints(indexT, pt0, pt1, pt2);

	TeBox boxT1;
	updateBox(boxT1, pt0);
	updateBox(boxT1, pt1);
	updateBox(boxT1, pt2);

	triangleSet_[indexT].setBox(boxT1);
	indexTriang_->insert(boxT1, indexT);

	TeBox oldboxT = triangleSet_[t].box();
	indexTriang_->remove(oldboxT, t);

//10. Defina as arestas de t como sendo an2, a2 e an0,
	long a2 = triangleSet_[t].edgeAt(2); // retorna aresta 2 de t
	triangleSet_[t].setEdges(an2, a2, an0);

	this->trianglePoints(t, pt0, pt1, pt2);

	TeBox boxT2;
	updateBox(boxT2, pt0);
	updateBox(boxT2, pt1);
	updateBox(boxT2, pt2);

	triangleSet_[t].setBox(boxT2);
	indexTriang_->insert(boxT2, t);

	triangles.insert(t);
	triangles.insert(t1);
	triangles.insert(t2);

	return true;
}

long
TeTin::duplicateTriangle(long t, long n, long v, std::set<long>& triangles)
{
	TeCoord2D pt0, pt1, pt2;
	TeBox oldboxT = triangleSet_[t].box();

// Seja t o triÃ¢ngulo que contÃ©m cuja n-Ã©sima aresta an Ã© a aresta 
// prÃ³xima a amostra s ( n E {0,1,2} ). A aresta an conecta os vÃ©rtices 
// vn e vj, a aresta aj conecta os vÃ©rtices vj e vk e a aresta ak conecta
// os vÃ©rtices vk e vn, sendo j o resto da divisÃ£o de n+1 por 3 e k o resto
// da divisÃ£o de n+2 por 3.
	long edges[3];
	triangleSet_[t].edges(edges[0],edges[1],edges[2]);

	long an = edges [ n ];
	long aj = edges [ (n+1)%3 ];
	long ak = edges [ (n+2)%3 ];

	long vn = this->triangleVertexAt( t, n );

//	1. Crie o vÃ©rtice v com os dados da amostra s,
//	2. Defina o triÃ¢ngulo tv que compartilha a aresta an com t,
	long tv = 0;
	if (edgeSet_[an].left() == t)
		tv = edgeSet_[an].right();
	else if (edgeSet_[an].right() == t)
		tv = edgeSet_[an].left();

//	3. Defina o vÃ©rtice vop do triÃ¢ngulo t que nÃ£o Ã© conectado a aresta an,
	long vop = this->triangleVertexAt ( t, (n+2)%3 );

//	4. Crie o novo triÃ¢ngulos t1,
	long t1 = triangleSet_.size();

//	5. Crie a nova aresta an0 que conecta os vÃ©rtices v e vn e
//	   tem t1 e tv como os triÃ¢ngulos que compartilham a aresta.
	edgeSet_.add( TeTinEdge (v, vn, t1, tv, NORMALEDGE) );
	long an0 = edgeSet_.size()-1;

//	6. Crie a nova aresta an1 que conecta os vÃ©rtices v e vop e
//	   tem t e t1 como os triÃ¢ngulos que compartilham a aresta.
	edgeSet_.add( TeTinEdge (v, vop, t, t1, NORMALEDGE) );
	long an1 = edgeSet_.size()-1;

//	7. Modifique a aresta an para conectar o vÃ©rtice v ao invÃ©s de vn,
	edgeSet_[an].exchangeVertex( vn, v );
	vertexSet_[vn].setOneEdge ( an0 );

	indexTriang_->remove(oldboxT, t);

//	8. Defina as arestas de t como sendo an, aj e an1,
	triangleSet_[t].setEdges( an, aj, an1 );

//	9. Modifique a aresta ak para compartilhar o triÃ¢ngulo t1 ao invÃ©s de t,
	edgeSet_[ak].exchangeTriangle( t, t1 );

	//Atualizar triangulo t na RTree
	this->trianglePoints(t, pt0, pt1, pt2);

	TeBox boxT;
	updateBox(boxT, pt0);
	updateBox(boxT, pt1);
	updateBox(boxT, pt2);

	triangleSet_[t].setBox(boxT);
	indexTriang_->insert(boxT, t);

//	10. Defina as arestas de t1 como sendo an0, an1 e ak,
	triangleSet_.add ( TeTinTriangle (an0, an1, ak) );

	long indexT = triangleSet_.size() - 1;
	this->trianglePoints(indexT, pt0, pt1, pt2);

	TeBox boxT1;
	updateBox(boxT1, pt0);
	updateBox(boxT1, pt1);
	updateBox(boxT1, pt2);

	triangleSet_[indexT].setBox(boxT1);
	indexTriang_->insert(boxT1, indexT);

	triangles.insert(t);
	triangles.insert(t1);

	return an0;
}

bool
TeTin::dupNeighTriangle(long tv, long an0, long v, std::set<long>& triangles)
{
	TeCoord2D pt0, pt1, pt2;

//	11.1. Crie o novo triÃ¢ngulo t2,
	long t2 = triangleSet_.size();

//	11.2. Defina a aresta av do triÃ¢ngulo tv que contÃ©m o vÃ©rtice vn
//	      (obs: sÃ³ hÃ¡ uma aresta porque a outra foi modificada),
	long vn = edgeSet_[an0].to(); //Due to assembly, dangerous
	if ( vn == v )
		vn = edgeSet_[an0].from();

	TeBox oldboxT = triangleSet_[tv].box();

	long edges[3];
	triangleSet_[tv].edges(edges[0],edges[1],edges[2]);
	long i;
	for (i = 0; i < 3; i++)
	{
		if ( (edgeSet_[edges[i]].from() == vn) ||
			 (edgeSet_[edges[i]].to() == vn) )
			break;
	}
	if (i == 3)
		return false;
	long av = edges[i];

//	11.3. Defina as outras arestas de tv como sendo av1 e av2.
	long av1 = edges[ (i+1)%3];
	long av2 = edges[ (i+2)%3];

//	11.4. Defina o vÃ©rtice vvo conectado a vn por meio da aresta av,
	long vvo;
	if (edgeSet_[av].from() == vn)
		vvo = edgeSet_[av].to();
	else
		vvo = edgeSet_[av].from();

//	11.5. Crie a nova aresta an2 que conecta os vÃ©rtices v e vvo e
//	      tem t e t2 como os triÃ¢ngulos que compartilham a aresta.
	edgeSet_.add( TeTinEdge (v, vvo, tv, t2, NORMALEDGE) );
	long an2 = edgeSet_.size()-1;

//	11.6. Troque o triÃ¢ngulo tv por t2 na aresta av,
	edgeSet_[av].exchangeTriangle( tv, t2 );

//	11.7. Troque o triÃ¢ngulo tv por t2 na aresta an0,
	edgeSet_[an0].exchangeTriangle( tv, t2 );

	indexTriang_->remove(oldboxT, tv);

//	11.8. Defina as arestas de tv como sendo an2, av1 e av2.
	triangleSet_[tv].setEdges( an2, av1, av2 );

	this->trianglePoints(tv, pt0, pt1, pt2);

	TeBox boxT;
	updateBox(boxT, pt0);
	updateBox(boxT, pt1);
	updateBox(boxT, pt2);

	triangleSet_[tv].setBox(boxT);
	indexTriang_->insert(boxT, tv);

//	11.9. Defina as arestas de t2 como sendo an0, av e an2,
	triangleSet_.add ( TeTinTriangle ( an0, av, an2 ) );

	long indexT = triangleSet_.size() - 1;
	this->trianglePoints(indexT, pt0, pt1, pt2);

	TeBox boxT1;
	updateBox(boxT1, pt0);
	updateBox(boxT1, pt1);
	updateBox(boxT1, pt2);

	triangleSet_[indexT].setBox(boxT1);
	indexTriang_->insert(boxT1, indexT);

	triangles.insert(tv);
	triangles.insert(t2);

	return true;
}

void
TeTin::testDelaunayForVertex(long v, std::set<long>& triangles)
{
	while ( triangles.size() )
	{
		std::set<long>::iterator i = triangles.begin();
		long t = *i;
		triangles.erase(t);
		this->testDelaunayAt(t, v, triangles);
	}
}

bool
TeTin::testDelaunayAt(long t, long v, std::set<long>& triangles)
{
	//	Retrieve line of triangle common to neighbor triangle
	long e = this->edgeOppVertex (t, v);

//	Retrieve neighbour triangle (tviz) pointer
	long tviz = edgeSet_[e].left();
	if ( edgeSet_[e].left() == t )
		tviz = edgeSet_[e].right();
	if ( tviz == -1 )
		return false;

//	Define base triangle (tri) center point
	TeCoord2D vert[3];
	this->trianglePoints(t, vert[0], vert[1], vert[2]);

//	Find opposite point to base triangle (tri) inside neighbour (tviz)
	long vo = this->vertexOppEdge ( tviz, e );
	TeCoord2D pto = vertexSet_[vo].location();

//	To avoid overflow
	TeCoord2D ptmin( TeMAXFLOAT, TeMAXFLOAT );
	long i;
	for (i = 0; i < 3; i++)
		if ( vert[i] < ptmin )
			ptmin = vert[i];

	if ( ! ptmin.tooBig() )
	{
		TeCoord2D mptmin ( -ptmin.x(), -ptmin.y() );
		for (i = 0; i < 3; i++)
			vert[i] += mptmin;
		pto += mptmin;
	}

	TeCoord2D ptc;
	if ( ! TeFindTriangleCenter( vert[0], vert[1], vert[2], ptc ) )
		return false;

//	Calculate base triangle (tri) radius
	double xaux = ptc.x() - vert[0].x();
	double yaux = ptc.y() - vert[0].y();
	double raio2 = xaux * xaux + yaux * yaux;

//	Calculate distance from opposite point (tviz) to center point (tri)
	xaux = ptc.x() - pto.x();
	yaux = ptc.y() - pto.y();
	double dist2 = xaux * xaux + yaux * yaux; 

/*	float fr = (float)sqrt(raio2);
	float fd = (float)sqrt(dist2);
	if (fr <= fd)
		return false; */

	if (raio2 <= dist2)
		return false;

	//	If not, change edge between tri and ntri
	bool status = this->swapEdges(t, tviz, e);
	if (status == true)
	{
		triangles.insert(t);
		triangles.insert(tviz);
	}
	return status;
}

bool
TeTin::testDelaunay(long t1, long t2, long t3)
{
	long neigh[3];
	this->triangle3Neighbors(t1, neigh[0], neigh[1], neigh[2]);

	int k;
	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t2) && (neigh[k] != t3))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t1, k) == FALSE)
	this->testDelaunay(t1, k);

	this->triangle3Neighbors(t2, neigh[0], neigh[1], neigh[2]);

	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t1) && (neigh[k] != t3))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t2, k) == FALSE)
	this->testDelaunay(t2, k);

	this->triangle3Neighbors(t3, neigh[0], neigh[1], neigh[2]);

	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t1) && (neigh[k] != t2))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t2, k) == FALSE)
	this->testDelaunay(t3, k);

	return true;
}

bool
TeTin::testDelaunay(long t1, long t2, long t3, long t4)
{
	long neigh[3];
	this->triangle3Neighbors(t1, neigh[0], neigh[1], neigh[2]);

	int k;
	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t2) && (neigh[k] != t3) && (neigh[k] != t4))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t1, k) == FALSE)
	this->testDelaunay(t1, k);

	this->triangle3Neighbors(t2, neigh[0], neigh[1], neigh[2]);

	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t1) && (neigh[k] != t3) && (neigh[k] != t4))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t1, k) == FALSE)
	this->testDelaunay(t2, k);

	this->triangle3Neighbors(t3, neigh[0], neigh[1], neigh[2]);

	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t1) && (neigh[k] != t2) && (neigh[k] != t4))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t1, k) == FALSE)
	this->testDelaunay(t3, k);

	this->triangle3Neighbors(t4, neigh[0], neigh[1], neigh[2]);

	for (k = 0; k < 3; k++)
	{
		if ((neigh[k] != t1) && (neigh[k] != t2) && (neigh[k] != t3))
			break;
	}

	if (k == 3)
		return false;

	//if (IsNeighborOnIsoOrBreakline(t1, k) == FALSE)
	this->testDelaunay(t4, k);

	return true;
}

bool
TeTin::testDelaunay(long e)
{
	long tv = edgeSet_[e].left();

	if (tv == -1)
		return false;

	long edges[3];
	triangleSet_[tv].edges(edges[0], edges[1], edges[2]);

	int j;
	for (j = 0; j < 3; j++)
	{
		if (edges[j] == e)
			break;
	}

	bool test = false;

	if (j != 3)
		test = this->testDelaunay(tv, j);

	return test;
}

bool
TeTin::testDelaunay(long t, long v)
{
	if (t == -1)
		return false;

	//	Retrieve neighbour triangle (tviz) pointer
	long neigh[3];
	this->triangle3Neighbors(t, neigh[0], neigh[1], neigh[2]);

	if (neigh[v] == -1)
		return false;

	//	Retrieve line of triangle common to neighbor triangle
	long e = triangleSet_[t].edgeAt(v);

	//	Define base triangle (tri) center point
	TeCoord2D vert[3];
	this->trianglePoints(t, vert[0], vert[1], vert[2]);

	//	Find opposite point to base triangle (tri) inside neighbour (tviz)
	long opvertex = this->vertexOppEdge(neigh[v], e);

	double minx = TeMAXFLOAT;
	double miny = TeMAXFLOAT;

	for (int i = 0; i < 3; i++)
	{
		if (vert[i].x() < minx)
			minx = vert[i].x();
		if (vert[i].y() < miny)
			miny = vert[i].y();
	}

	double x0, y0;

	if ((minx < TeMAXFLOAT) && (miny < TeMAXFLOAT))
	{
		for (int i = 0; i < 3; i++)
		{
			vert[i].x(vert[i].x() - minx);
			vert[i].y(vert[i].y() - miny);
		}

		x0 = vertexSet_[opvertex].location().x() - minx;
		y0 = vertexSet_[opvertex].location().y() - miny;
	}
	else
	{
		x0 = vertexSet_[opvertex].location().x();
		y0 = vertexSet_[opvertex].location().y();
	}

	TeCoord2D ptc;
	if ( ! TeFindTriangleCenter( vert[0], vert[1], vert[2], ptc ) )
		return false;

	double xaux = ptc.x() - vert[0].x();
	double yaux = ptc.y() - vert[0].y();
	double raio2 = (xaux * xaux) + (yaux * yaux);

	//	Calculate distance from opposite point (tviz) to center point (tri)
	xaux = ptc.x() - x0;
	yaux = ptc.y() - y0;
	double dist2 = (xaux * xaux) + (yaux * yaux);
	double fr = sqrt(raio2);
	double fd = sqrt(dist2);

	if (fr <= fd)
		return false;


	bool status = this->swapEdges(t, neigh[v], e);

	return status;
}

bool
TeTin::createDelaunay()
{
	long contr = 0;
	long npoly = -1;

	for (long t = 1; t < triangleSet_.size(); t++)
	{
		if (t > npoly)
			npoly = t;

		if (this->generateDelaunay(npoly, npoly, contr) == false)
		{
			return false;
		}
	}

	return true;
}

bool
TeTin::generateDelaunay(long nt, long ntbase, long& contr)
{
	contr++;

	if (contr > triangleSet_.size())
		return false;

	for (long j = 0; j < 3; j++)
	{
		long neigh[3];
		this->triangle3Neighbors(nt, neigh[0], neigh[1], neigh[2]);

		long aux = neigh[j];
		if (aux == nt)
			continue;

		//Test with each neighbor, if it exists
		if (aux == -1)
			continue;

		if (this->isNeighborOnIsoOrBreakline(nt, j))
			continue;

		if (this->testDelaunay(nt, j))
		{
			//If changed:
			if (ntbase > aux)
				this->generateDelaunay(aux, ntbase, contr);

			j = -1;
		}
	}

	return true;
}

void
TeTin::vertexExchange(long oldVertex, long newVertex)
{
	std::list<long> edges;
	this->vertexEdges(oldVertex, edges);

	long e;
	std::list<long>::iterator it = edges.begin();
	while (it != edges.end())
	{
		e = *it;
		edgeSet_[e].exchangeVertex(oldVertex, newVertex);

		it++;
	}

	double x = vertexSet_[oldVertex].location().x();
	double y = vertexSet_[oldVertex].location().y();
	double z = vertexSet_[oldVertex].value();

	vertexSet_[newVertex].location().setXY(x, y);
	vertexSet_[newVertex].value(z);
	vertexSet_[newVertex].setOneEdge(e);
}

bool
TeTin::createSmallerAngleTriangulation()
{
	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Creating triangulation using Smaller Angle method...");
		TeProgress::instance()->setTotalSteps(triangleSet_.size());
	}

	int step = -1;
	long neighbor[3];

	for (long t = 0; t < triangleSet_.size(); t++)
	{
		this->triangle3Neighbors(t, neighbor[0], neighbor[1], neighbor[2]);

		for (long v = 0; v < 3; v++)
		{
			if (this->testSmallerAngleAt(t, v))
			{//Se triangulo t e seu vizinho tv forem alterados
				if (neighbor[v] < t)
				{//Se vizinho ja foi testado
					t = neighbor[v] - 1;
					v = 3;
				}
				else
				{//Se vizinho ainda nao foi testado
					v = -1;
				}
			}
		}

		if (t > step)
		{
			step = t;
			TeProgress::instance()->setProgress(step);
		}

		if (TeProgress::instance()->wasCancelled())
		{
			errorMessage_ = "Operation was cancelled.";
			TeProgress::instance()->reset();

			return false;
		}
	}

	TeProgress::instance()->reset();

	return true;
}

bool
TeTin::testSmallerAngleAt(long t, long v)
{
	if (t == -1)
		return false;

	//Retrieve neighbour triangle (tviz) pointer
	long neigh[3];
	this->triangle3Neighbors(t, neigh[0], neigh[1], neigh[2]);

	if (neigh[v] == -1)
		return false;

	long tviz = neigh[v];

	//Retrieve line of triangle common to neighbor triangle
	long e = triangleSet_[t].edgeAt(v);

	long vertid1 = edgeSet_[e].to();
	long vertid2;
	if (vertid1 > edgeSet_[e].from())
	{
		vertid1 = edgeSet_[e].from();
		vertid2 = edgeSet_[e].to();
	}
	else
		vertid2 = edgeSet_[e].from();

	if ((vertexSet_[vertid1].value() >= TeMAXFLOAT) || (vertexSet_[vertid2].value() >= TeMAXFLOAT))
		return false;

	long vertice[3], vertice1[3];
	this->triangleVertices(t, vertice[0], vertice[1], vertice[2]);
	this->triangleVertices(tviz, vertice1[0], vertice1[1], vertice1[2]);

	std::vector<TeTinVertex> vertexVec1, vertexVec2;
	for (int i = 0; i < 3; i++)
	{
		TeTinVertex vertex = vertexSet_[vertice[i]];
		TeTinVertex vertex1 = vertexSet_[vertice1[i]];

		if ((vertex.value() >= TeMAXFLOAT) || (vertex1.value() >= TeMAXFLOAT))
			return false;

		vertexVec1.push_back(vertexSet_[vertice[i]]);
		vertexVec2.push_back(vertexSet_[vertice1[i]]);
	}

	double nvector[3], nvector1[3];

	//Define base triangle (tri) normal vector
	this->normalTriangle(vertexVec1, nvector); 
	this->normalizeVector(nvector);

	//Define opposite triangle (nviz) normal vector
	this->normalTriangle(vertexVec2, nvector1);
	this->normalizeVector(nvector1);

	//Calculate cosine between triangles
	double cos1 = nvector[0]*nvector1[0]+nvector[1]*nvector1[1]+nvector[2]*nvector1[2];

	//Find opposite point to common edge on base triangle
	vertid1 = this->oppositeVertex(t, e);

	//Find opposite point to common edge on neighbor triangle
	vertid2 = this->oppositeVertex(tviz, e);

	//Fill in first possible triangle points
	vertexVec1[0].location().x(vertexSet_[vertid1].location().x());
	vertexVec1[0].location().y(vertexSet_[vertid1].location().y());
	vertexVec1[0].value(vertexSet_[vertid1].value());
	vertexVec1[1].location().x(vertexSet_[vertid2].location().x());
	vertexVec1[1].location().y(vertexSet_[vertid2].location().y());
	vertexVec1[1].value(vertexSet_[vertid2].value());
	vertexVec1[2].location().x(vertexSet_[edgeSet_[e].from()].location().x());
	vertexVec1[2].location().y(vertexSet_[edgeSet_[e].from()].location().y());
	vertexVec1[2].value(vertexSet_[edgeSet_[e].from()].value());

	//Fill in second possible triangle points
	vertexVec2[0].location().x(vertexSet_[vertid1].location().x());
	vertexVec2[0].location().y(vertexSet_[vertid1].location().y());
	vertexVec2[0].value(vertexSet_[vertid1].value());
	vertexVec2[1].location().x(vertexSet_[vertid2].location().x());
	vertexVec2[1].location().y(vertexSet_[vertid2].location().y());
	vertexVec2[1].value(vertexSet_[vertid2].value());
	vertexVec2[2].location().x(vertexSet_[edgeSet_[e].to()].location().x());
	vertexVec2[2].location().y(vertexSet_[edgeSet_[e].to()].location().y());
	vertexVec2[2].value(vertexSet_[edgeSet_[e].to()].value());

	//Define first possible triangle normal vector
	this->normalTriangle(vertexVec1, nvector);
	this->normalizeVector(nvector);

	//Define second possible triangle normal vector
	this->normalTriangle(vertexVec2, nvector1);
	this->normalizeVector(nvector1);

	//Calculate cosine between triangles
	double cos2 = nvector[0]*nvector1[0]+nvector[1]*nvector1[1]+nvector[2]*nvector1[2];

	//If new triangles normal vector angle difference smaller than old triangle's
	bool status = false;

	if ((float)cos2 > (float)cos1)
		status = this->swapEdges(t, tviz, e);

	return status;
}

bool
TeTin::createIsolinesTriangulation()
{
	TeTinVertexSet vertSet;
	std::vector<long> svertex;

	for (int v = 0; v < vertexSet_.size(); v++)
		svertex.push_back(0);

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Updating triangulation using With Isolines method...");
		TeProgress::instance()->setTotalSteps(edgeSet_.size() + (vertexSet_.size() - 2));
	}

	int step;

	for (int e = 0; e < edgeSet_.size(); e++)
	{
		step = e;
		TeProgress::instance()->setProgress(step);

		if (edgeSet_[e].from() == -1)
			continue;

		long v0 = edgeSet_[e].from();
		long v1;

		if (v0 > edgeSet_[e].to())
		{
			v0 = edgeSet_[e].to();
			v1 = edgeSet_[e].from();
		}
		else
			v1 = edgeSet_[e].to();

		//if ((vertexSet_[v0].type() == DELETEDVERTEX) || (vertexSet_[v0].type() == NORMALVERTEX) ||
		//	(vertexSet_[v0].type() == ONBOXVERTEX) || (vertexSet_[v0].type() == CONTOURLASTVERTEX) ||
		//	(vertexSet_[v0].type() == ONBREAKLINEVERTEX) || (vertexSet_[v0].type() == BREAKLINEFIRSTVERTEX) ||
		//	(vertexSet_[v0].type() == BREAKLINELASTVERTEX))
		if ((vertexSet_[v0].type() != ONCONTOURVERTEX) &&
			(vertexSet_[v0].type() != CONTOURFIRSTVERTEX))
		{
			svertex[v0] = 1;
			continue;
		}

		long v = v0 + 1;
		if (v == v1)
			svertex[v0] = 1;

		if (TeProgress::instance()->wasCancelled())
		{
			errorMessage_ = "Operation was cancelled.";
			TeProgress::instance()->reset();

			return false;
		}
	}

	bool modified = false;

	for (long v0 = 1; v0 < vertexSet_.size() - 1; v0++)
	{
		step++;
		TeProgress::instance()->setProgress(step);

		//if ((vertexSet_[v0].type() == DELETEDVERTEX) || (vertexSet_[v0].type() == NORMALVERTEX) ||
		//	(vertexSet_[v0].type() == ONBOXVERTEX) || (vertexSet_[v0].type() == CONTOURLASTVERTEX) ||
		//	(vertexSet_[v0].type() == ONBREAKLINEVERTEX) || (vertexSet_[v0].type() == BREAKLINEFIRSTVERTEX) ||
		//	(vertexSet_[v0].type() == BREAKLINELASTVERTEX))
		if ((vertexSet_[v0].type() != ONCONTOURVERTEX) &&
			(vertexSet_[v0].type() != CONTOURFIRSTVERTEX))
			continue;

		if (svertex[v0] == 1)
			continue;

		long v1 = v0 + 1;

		if ((vertexSet_[v1].type() != ONCONTOURVERTEX) && (vertexSet_[v1].type() != CONTOURLASTVERTEX))
			continue;

		//Find triangle edge that intersects isoline segment
		std::list<long> edges;
		this->vertexOppEdges(v0, edges);

		TeCoord2D pt0 = vertexSet_[v0].location();
		TeCoord2D pt1 = vertexSet_[v1].location();
		TeCoord2D ptf;

		long edgeaux = -1;

		std::list<long>::iterator it = edges.begin();
		while (it != edges.end())
		{
			long e1 = *it;
			TeCoord2D pte0 = vertexSet_[edgeSet_[e1].to()].location();
			TeCoord2D pte1 = vertexSet_[edgeSet_[e1].from()].location();

			if (TeSegmentsIntersectPoint(pt0, pt1, pte0, pte1, ptf))
			{
				//Save first edge of triangulation that intersects isoline segment
				edgeaux = e1;
				break;
			}

			it++;
		}

		if (edgeaux == -1)
		{
			edges.clear();
			continue;
		}

		long tright = edgeSet_[edgeaux].right();
		long tleft = edgeSet_[edgeaux].left();
		long trvert[3], tlvert[3];
		this->triangleVertices(tright, trvert[0], trvert[1], trvert[2]);
		this->triangleVertices(tleft, tlvert[0], tlvert[1], tlvert[2]);

		//Find triangle at opposite side of vertex, compared to one edge
		long ntri = -1;
		for (int i = 0; i < 3; i++)
		{
			if (trvert[i] == v0)
			{
				ntri = tleft;
				break;
			}

			if (tlvert[i] == v0)
			{
				ntri = tright;
				break;
			}
		}

		if (ntri == -1)
		{
			edges.clear();
			svertex.clear();

			TeProgress::instance()->reset();

			return false;
		}

		long tvert = this->oppositeVertex(tright, edgeaux);
		long vvert = this->oppositeVertex(tleft, edgeaux);
		long v = edgeSet_[edgeaux].from();
		long vaux;

		if (v > edgeSet_[edgeaux].to())
		{
			v = edgeSet_[edgeaux].to();
			vaux = edgeSet_[edgeaux].from();
		}
		else
			vaux = edgeSet_[edgeaux].to();

		long vaux1 = v + 1;

		if ((tvert == v1) || (vvert == v1))
		{
			if (((vertexSet_[v].type() != CONTOURFIRSTVERTEX) && ( vertexSet_[v].type() != ONCONTOURVERTEX)) ||
				(vaux1 != vaux))
			{
				if (this->swapEdges(edgeSet_[edgeaux].left(), edgeSet_[edgeaux].right(), edgeaux))
				{
					modified = true;
					continue;
				}
			}
		}

		TeTinVertex ptaux;
		ptaux.location().setXY(ptf.x(), ptf.y());
		ptaux.value(vertexSet_[v0].value());
		ptaux.setType(CONTOURFIRSTVERTEX);
		vertSet.add(ptaux);

		vaux = v0;
		long oldedge = edgeaux;

		while (vaux != v1)
		{
			//Test if reached final isoline vertex
			long vedge[3];
			triangleSet_[ntri].edges(vedge[0], vedge[1], vedge[2]);

			for (int i = 0; i < 3; i++)
			{
				if (vedge[i] == oldedge)
					continue;

				if ((edgeSet_[vedge[i]].to() == v1) || (edgeSet_[vedge[i]].from() == v1))
				{
					vaux = v1;
					break;
				}
			}

			if (vaux == v1)
				break;

			TeCoord2D pt;

			//Find intersection point between triangle edge and isoline segment
			long edge1;
			int i;
			for (i = 0; i < 3; i++)
			{
				if (vedge[i] == oldedge) //if previous edge
					continue;

				edge1 = vedge[i];

				if ((edgeSet_[edge1].to() == v0) || (edgeSet_[edge1].from() == v0))
					continue;

				TeCoord2D pte0 = vertexSet_[edgeSet_[edge1].to()].location();
				TeCoord2D pte1 = vertexSet_[edgeSet_[edge1].from()].location();

				if (TeSegmentsIntersectPoint(pt0, pt1, pte0, pte1,pt))
					break;
			}

			if (i == 3)
				break;

			//Insert intersected point to point list
			ptaux = TeTinVertex();
			ptaux.location().setXY(pt.x(), pt.y());
			ptaux.value(vertexSet_[v0].value());
			ptaux.setType(ONCONTOURVERTEX);
			vertSet.add(ptaux);

			edgeaux = edgeSet_[edge1].to();
			oldedge = edge1;

			if (edgeSet_[edge1].left() == ntri)
				ntri = edgeSet_[edge1].right();
			else
				ntri = edgeSet_[edge1].left();

			modified = true;
		}

		TeTinVertex ptaux1 = vertSet.last();
		if (ptaux1.type() == ONCONTOURVERTEX)
			ptaux1.setType(CONTOURLASTVERTEX);
		else if (ptaux1.type() == CONTOURFIRSTVERTEX)
			ptaux1.setType(NORMALVERTEX);

		if (vertexSet_[v0].type() == CONTOURFIRSTVERTEX)
			vertexSet_[v0].setType(NORMALVERTEX);
		else
			vertexSet_[v0].setType(CONTOURLASTVERTEX);

		if (vertexSet_[v1].type() == ONCONTOURVERTEX)
			vertexSet_[v1].setType(CONTOURFIRSTVERTEX);
		else
			vertexSet_[v1].setType(NORMALVERTEX);

		edges.clear();

		if (TeProgress::instance()->wasCancelled())
		{
			svertex.clear();
			errorMessage_ = "Operation was cancelled.";
			TeProgress::instance()->reset();

			return false;
		}
	}

	svertex.clear();

	TeProgress::instance()->reset();

	if (vertSet.size() == 0)
		return modified;

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Inserting new points...");
		TeProgress::instance()->setTotalSteps(vertSet.size());
	}

	for (int i = 0; i < vertSet.size(); i++)
	{
		TeProgress::instance()->setProgress(i);

		double x = vertSet[i].location().x();
		double y = vertSet[i].location().y();
		double value = vertSet[i].value();
		verttype_ = vertSet[i].type();

		this->insertPoint(x, y, value);

		if (TeProgress::instance()->wasCancelled())
		{
			clear();
			vertSet.clear();

			errorMessage_ = "Operation was cancelled.";

			TeProgress::instance()->reset();

			return false;
		}
	}

	TeProgress::instance()->reset();

	return modified;
}

long
TeTin::oppositeVertex(long t, long e)
{
	long vto = edgeSet_[e].to();
	long vfrom = edgeSet_[e].from();

	long vert[3];
	this->triangleVertices(t, vert[0], vert[1], vert[2]);

	int i;
	for (i = 0; i < 3; i++)
	{
		if ((vto == vert[i]) || (vfrom == vert[i]))
			continue;

		break;
	}

	if (i == 3)
		return 0;

	return vert[i];
}

bool
TeTin::swapEdges( long t, long tv, long ai )
{
	TeCoord2D pt0, pt1, pt2;

	if ( tv == -1 )
		return false;
// Seja t o triÃ¢ngulo que contÃ©m cuja i-Ã©sima aresta ai Ã© a aresta
//	que se deseja alterar ( i E {0,1,2} ). A aresta ai conecta os
//	vÃ©rtices vi e vj, a aresta aj conecta os vÃ©rtices vj e vk e a
//	aresta ak conecta os vÃ©rtices vk e vi, sendo j o resto da divisÃ£o
//	de i+1 por 3 e k o resto da divisÃ£o de i+2 por 3.

	long tedges[3];
	triangleSet_[t].edges ( tedges[0], tedges[1], tedges[2] );

	long vertex[3];
	this->triangleVertices ( t, vertex[0], vertex[1], vertex[2]);
	long i;
	for ( i= 0; i < 3; i++ )
		if ( tedges[i] == ai )
			break;

	if (i == 3)
		return false;

	long aj = tedges [ (i+1) % 3 ];
	long ak = tedges [ (i+2) % 3 ];

	long vi = vertex [ i ];
	long vj = vertex [ (i+1) % 3 ];
	long vk = vertex [ (i+2) % 3 ];

// Seja tv o triÃ¢ngulo que compartilha a aresta ai com t. O vÃ©rtice de
//	tv que nÃ£o Ã© conectado pela aresta ai Ã© o vÃ©rtice vn. As outras
//	arestas do triÃ¢ngulo tv sÃ£o am que conecta os vÃ©rtices vi e vn e a
//	aresta an conecta os vÃ©rtices vn e vj.
	
	long vn = this->vertexOppEdge ( tv, ai );

	long tvedges [3];
	triangleSet_[tv].edges ( tvedges[0], tvedges[1], tvedges[2] );

	long am, an = 0;
	for ( long j = 0; j < 3; j++ )
	{
		if ( tvedges [ j ] == ai )
			continue;

		if ( edgeSet_[tvedges[j]].from() == vn )
		{
			if ( edgeSet_[tvedges[j]].to() == vi )
				am = tvedges [ j ];
			else if ( edgeSet_[tvedges[j]].to() == vj )
				an = tvedges [ j ];
		}
		else if ( edgeSet_[tvedges[j]].to() == vn )
		{
			if ( edgeSet_[tvedges[j]].from() == vi )
				am = tvedges [ j ];
			else if ( edgeSet_[tvedges[j]].from() == vj )
				an = tvedges [ j ];
		}
	}

	TeCoord2D ptaux;
	double tol;// = TePrecision::instance().precision();
	if (minedgesize_ > 0)
		tol = minedgesize_;
	else
		tol = TePrecision::instance().precision();
	if ( TePerpendicularDistance(vertexSet_[vk].location(), vertexSet_[vn].location(),
								 vertexSet_[vi].location(), ptaux) < tol )
		return false;
	if ( TePerpendicularDistance(vertexSet_[vk].location(), vertexSet_[vn].location(),
								 vertexSet_[vj].location(), ptaux) < tol )//TePrecision::instance().precision() )
		return false;

//	1. Se o segmento de reta formado conectando o vÃ©rtice vk do
//	   triÃ¢ngulo t ao vÃ©rtice vn de tv intersecta a aresta ai, faÃ§a:
	TeCoord2D pi;
	if (TeSegmentsIntersectPoint(vertexSet_[vi].location(), vertexSet_[vj].location(), vertexSet_[vk].location(), vertexSet_[vn].location(), pi) == false )
		return false;

//		1.1. Troque na aresta ai o vÃ©rtice vi pelo vertice vk e o
//			 vÃ©rtice vj pelo vÃ©rtice vn.
	edgeSet_[ai].exchangeVertex ( vi, vk );
	edgeSet_[ai].exchangeVertex ( vj, vn );
	//if (edgeSet_[ai].to() == vi)
	//	edgeSet_[ai].setVertices(vn, vk);
	//else if (edgeSet_[ai].to() == vj)
	//	edgeSet_[ai].setVertices(vk, vn);

	if ( vertexSet_[vi].oneEdge() == ai )
		vertexSet_[vi].setOneEdge ( ak );
	if ( vertexSet_[vj].oneEdge() == ai )
		vertexSet_[vj].setOneEdge ( aj );

//		1.2. Troque na aresta an o triÃ¢ngulo tv pelo triÃ¢ngulo t.
	edgeSet_[an].exchangeTriangle( tv , t );
	//if (edgeSet_[an].right() == tv)
	//	edgeSet_[an].setTriangles(edgeSet_[an].left(), t);
	//else
	//	edgeSet_[an].setTriangles(t, edgeSet_[an].right());

//		1.3. Troque na aresta ak o triÃ¢ngulo t pelo triÃ¢ngulo tv.
	edgeSet_[ak].exchangeTriangle( t, tv );
	//if (edgeSet_[ak].right() == t)
	//	edgeSet_[ak].setTriangles(edgeSet_[ak].left(), tv);
	//else
	//	edgeSet_[ak].setTriangles(tv, edgeSet_[ak].right());

//		1.4. Troque no triÃ¢ngulo t a aresta ai pela aresta an e a
//			 aresta ak pela aresta ai.
	for ( i = 0; i < 3; i++ )
		if ( tedges[i] == ai )
			break;

	TeBox oldboxT = triangleSet_[t].box();
	indexTriang_->remove(oldboxT, t);

	tedges[i] = an;
	tedges[(i+2)%3] = ai;
	triangleSet_[t].setEdges ( tedges[0], tedges[1], tedges[2] );

	this->trianglePoints(t, pt0, pt1, pt2);

	//Atualizar box do triangulo t na RTree
	TeBox boxT;
	updateBox(boxT, pt0);
	updateBox(boxT, pt1);
	updateBox(boxT, pt2);

	triangleSet_[t].setBox(boxT);
	indexTriang_->insert(boxT, t);

//		1.5. Troque no triÃ¢ngulo tv a aresta ai pela aresta ak
	for ( i = 0; i < 3; i++ )
		if ( tvedges[i] == ai )
			break;
	tvedges [i] = ak;

//		 e a aresta an pela aresta ai.
	for ( i = 0; i < 3; i++ )
		if (tvedges[i] == an )
			break;
	tvedges[i] = ai;

	TeBox oldboxT1 = triangleSet_[tv].box();
	indexTriang_->remove(oldboxT1, tv);

	triangleSet_[tv].setEdges ( tvedges[0], tvedges[1], tvedges[2] );

	this->trianglePoints(tv, pt0, pt1, pt2);

	TeBox boxT1;
	updateBox(boxT1, pt0);
	updateBox(boxT1, pt1);
	updateBox(boxT1, pt2);

	triangleSet_[tv].setBox(boxT1);
	indexTriang_->insert(boxT1, tv);
	
	return true;
}

bool
TeTin::edgesInterBySegment( TeCoord2D& pf, TeCoord2D& pn, std::list<long> &aiset, std::list<long> &viset )
{
//		Seja o segmento de reta r que conecta os pontos pf  e pn,
//1.	Defina o triÃ¢ngulo tf que contÃ©m o ponto pf,
	//long tf = this->triangleAt (pf);
	long tf = this->triangleAt(TeBox(pf, pf));
	if (tf == -1)
		return false;

//2.	Defina o vÃ©rtice vaux e aresta aaux, como sendo invÃ¡lidos,
	long vaux = -1;
	long aaux = -1;

//3.	Defina o conjunto A={a1, a2, a3} com as arestas de tf,
	std::vector<long> auxset(3);
	triangleSet_[tf].edges ( auxset[0], auxset[1], auxset[2] );
	std::list<long> aedges ( auxset.begin(), auxset.end() );

//4.	Defina o conjunto V={v1, v2, v3} com os vÃ©rtices de tf,
	this->triangleVertices ( tf, auxset[0], auxset[1], auxset[2]);
	std::list <long> vvertex ( auxset.begin(), auxset.end() );

//5.	Defina o conjunto T={tf},
	std::list<long> triset( 1, tf );
	std::set<long> visetaux;

//6.	Para cada vÃ©rtice vi de V, faÃ§a:
	std::list<long>::iterator vii;
	for (vii = vvertex.begin(); vii != vvertex.end(); vii++)
	{
		long vi = *vii;
		TeCoord2D ptvi = vertexSet_[vi].location();
		TeCoord2D pinter;

//6.1.	Se o vÃ©rtice vi coincide com pf, faÃ§a:
		if ( ptvi == pf )
		{
//6.1.1.	Redefina A={a1,...,an} com as n arestas opostas a vi,
			aedges.clear();
			this->vertexOppEdges(vi, aedges);

//6.1.2.	Redefina V={v1,...,vm} com os m vÃ©rtices dos triÃ¢ngulos que contÃ©m o vÃ©rtice vi,
			vvertex.clear();
			this->vertexOppVertices(vi, vvertex);

//6.1.3.	Redefina o conjunto T={t1,..., tm} com os m triÃ¢ngulos que compartilham o vÃ©rtice vi,
			triset.clear();
			this->vertexTriangles(vi, triset);

//6.1.4.	Redefina o vÃ©rtice auxiliar vaux como sendo vi,
			vaux = vi;
			visetaux.insert(vi);
//6.1.5.	VÃ¡ para 8,
			break;
		}
	}
	if ( vii == vvertex.end() )
	{
//7.	Para cada aresta ai de A, faÃ§a:
		for (std::list<long>::iterator aii = aedges.begin(); aii != aedges.end(); aii++)
		{
			long ai = *aii;
//7.1.	Se o ponto pf estÃ¡ sobre ai, faÃ§a:
			TeCoord2D pfr = vertexSet_[edgeSet_[ai].from()].location();
			TeCoord2D pto = vertexSet_[edgeSet_[ai].to()].location();
			TeCoord2D pinter;
			if ( TePerpendicularDistance(pfr, pto, pf, pinter) < TePrecision::instance().precision() )
			{
//7.1.1.	Defina o triÃ¢ngulo tv que compartilha ai com tf,
				long tv = edgeSet_[ai].right();
				if (tv == tf)
					tv = edgeSet_[ai].left();
				if (tv == -1)
					return false;
//7.1.2.	Redefina V={v1, v2, v3, v4} com os vÃ©rtices de tv e tf,
				vvertex.clear();
				this->triangleVertices ( tv, auxset[0], auxset[1], auxset[2]);
				vvertex.insert( vvertex.begin(), auxset.begin(), auxset.end() );
				this->triangleVertices ( tf, auxset[0], auxset[1], auxset[2]);
				vvertex.insert( vvertex.begin(), auxset.begin(), auxset.end() );
				vvertex.sort();
				vvertex.unique();
//7.1.3.	Redefina A={a1, a2, a3, a4} com as arestas de tv e tf diferentes de ai,
				aedges.clear();
				triangleSet_[tv].edges ( auxset[0], auxset[1], auxset[2] );
				aedges.insert( aedges.begin(), auxset.begin(), auxset.end() );
				aedges.remove(ai);
				triangleSet_[tf].edges ( auxset[0], auxset[1], auxset[2] );
				aedges.insert( aedges.begin(), auxset.begin(), auxset.end() );
				aedges.remove(ai);

//7.1.4.	Redefina o conjunto T={tv, tf},
				triset.push_back ( tv );
//7.1.5.	Redefina a aresta auxiliar aaux como sendo ai,
				aiset.push_back ( ai );
				aaux = ai;
//7.1.6.	VÃ¡ para 8,
				break;
			}
		}
	}

	for (;;)
	{
//8.	Para cada triÃ¢ngulo ti de T, faÃ§a:
		std::list<long>::iterator tii;
		for ( tii = triset.begin(); tii != triset.end(); tii++)
		{
			long ti = *tii;
//8.1.1.	Se o triÃ¢ngulo ti contÃ©m o ponto pn, termine o algoritmo.
			if ( this->triangleContainsPoint( ti, pn ) )
				return true;
		}
//9.	Para cada vÃ©rtice vi de V, faÃ§a:
		std::list<long>::iterator vii;
		for (vii = vvertex.begin(); vii != vvertex.end(); vii++)
		{
			long vi = *vii;
			TeCoord2D ptvi = vertexSet_[vi].location();
//9.1.	Se vi coincide com pn, faÃ§a:
			if ( ptvi == pn )
			{
//9.1.1.	Insira o vÃ©rtice vi no conjunto VI de vÃ©rtices intersectados,
				viset.push_back ( vi );
//9.1.2.	Termine o algoritmo.
				break;
			}
		}
		if ( vii != vvertex.end() )
			break; // do for (;;)

//10.	Para cada vÃ©rtice vi de V, faÃ§a:
		for ( vii = vvertex.begin(); vii != vvertex.end(); vii++)
		{
			long vi = *vii;
//10.1.	Se vi Ã© diferente de vaux e estÃ¡ sobre r, faÃ§a:
			if ( vi != vaux )
			{
				TeCoord2D ptvi = vertexSet_[vi].location();
				TeCoord2D pinter;
				if ( TePerpendicularDistance(pf, pn, ptvi, pinter) < TePrecision::instance().precision() )
				{
					TeBox box1;
					updateBox (box1, pf);
					updateBox (box1, pn);
					if (TeWithin (pinter, box1) )
					{
//10.1.1.	Redefina o conjunto A={a1,..., an} com as n arestas opostas a vi, excluindo as arestas de AI,
						aedges.clear();
						this->vertexOppEdges(vi, aedges);
						std::list<long>::iterator aisi;
						for ( aisi = aiset.begin(); aisi != aiset.end(); aisi++)
							aedges.remove ( *aisi );
//10.1.2.	Redefina o conjunto V={v1,..., vm} com os m vÃ©rtices dos triÃ¢ngulos que contÃ©m o vÃ©rtice vi,
//			excluindo os vertices de Vi,
						vvertex.clear();
						this->vertexOppVertices(vi, vvertex);
						std::list<long>::iterator visi;
						for ( visi = viset.begin(); visi != viset.end(); aisi++)
							viset.remove ( *visi );
//101.3.	Redefina o conjunto T={t1, ..., tk } com os k triÃ¢ngulos que contÃ©m o vÃ©rtice vi,
						triset.clear();
						this->vertexTriangles(vi, triset);
//101.4.	Insira o vÃ©rtice vi no conjunto VI de vÃ©rtices intersectados,
						viset.push_back ( vi );
//101.5.	Defina o conjunto Ac={a1,..., aj} com as j arestas que contÃ©m vi, diferentes de aaux,
//101.6.	Insira as arestas de Ac no conjunto AI de arestas intersectadas,
						this->vertexEdges(vi, aiset);
						aiset.sort();
						aiset.unique();
//10.7.	Redefina o vÃ©rtice auxiliar vaux como sendo vi,
						vaux = vi;
//10.8.	Retorne a 8,
						break;
					}
				}
			}
		}
		if ( vii != vvertex.end() )
			continue; // do for (;;)

//11.	Para cada aresta ai de A, faÃ§a:
		std::list<long>::iterator aii;
		for (aii = aedges.begin(); aii != aedges.end(); aii++)
		{
			long ai = *aii;
//11.1.	Se o ponto pn estÃ¡ sobre ai, faÃ§a:
			TeCoord2D pfr = vertexSet_[edgeSet_[ai].from()].location();
			TeCoord2D pto = vertexSet_[edgeSet_[ai].to()].location();
			TeCoord2D pinter;
			if ( TePerpendicularDistance(pfr, pto, pn, pinter) < TePrecision::instance().precision() )
			{
				TeBox box1;
				updateBox (box1, pfr);
				updateBox (box1, pto);
				if (TeWithin (pinter, box1) )
				{
//11.1.1.	Insira a aresta ai no conjunto AI de arestas intersectadas,
					aiset.push_back ( ai );
//11.1.2.	Termine o algoritmo.
					break;
				}
			}
		}
		if ( aii != aedges.end() )
			break; // do for (;;)

//12.	Para cada aresta ai de A, faÃ§a:
		for (aii = aedges.begin(); aii != aedges.end(); aii++)
		{
			long ai = *aii;
			TeCoord2D pinter;
			long vt = edgeSet_[ai].to();
			long vf = edgeSet_[ai].from();
//12.1.	Se ai intersecta r, faÃ§a:
			if ( TeSegmentsIntersectPoint(vertexSet_[vt].location(), vertexSet_[vf].location(), pf, pn, pinter) )
			{
//12.1.1.	Insira a aresta ai no conjunto AI de arestas intersectadas,
				aiset.push_back ( ai );
//12.1.2.	Defina os triÃ¢ngulos t1 e t2 que compartilham a aresta ai.
//12.1.3.	Se t2 estÃ¡ contido no conjunto T , entÃ£o faÃ§a taux=t1,
				long taux = edgeSet_[ai].left();
				if (taux == -1)
					return false;
				std::list<long>::iterator pos1 = find (triset.begin(), triset.end(), taux);
				if ( pos1 != triset.end() )
//12.1.4.	SenÃ£o, faÃ§a taux=t2,
					taux = edgeSet_[ai].right();
				if (taux == -1)
					return false;
//12.1.5.	Se o triÃ¢ngulo taux contÃ©m o ponto pn, termine o algoritmo.
				if ( this->triangleContainsPoint( taux, pn ) )
					return true;
//12.1.6.	Redefina o conjunto A={a1, a2} com as arestas do triÃ¢ngulo taux diferentes de ai,
				triangleSet_[taux].edges ( auxset[0], auxset[1], auxset[2] );
				aedges.clear();
				aedges.insert( aedges.begin(), auxset.begin(), auxset.end() );
				aedges.remove( ai );
//12.1.7.	Redefina o conjunto V={v1}. O vÃ©rtice v1 Ã© o vÃ©rtice do triÃ¢ngulo taux que nÃ£o estÃ¡
//			em nenhuma extremidade da aresta ai,
				long vaux1 = edgeSet_[ai].from();
				long vaux2 = edgeSet_[ai].to();
				this->triangleVertices ( taux, auxset[0], auxset[1], auxset[2]);
				vvertex.clear();
				vvertex.insert ( vvertex.begin(), auxset.begin(), auxset.end() );
				vvertex.remove ( vaux1 );
				vvertex.remove ( vaux2 );

//12.1.8.	Redefina o conjunto T={taux},
				triset.clear();
				triset.push_back ( taux );
//12.1.9.	Redefina aaux como sendo ai,
				aaux = ai;
//12.1.10.	Retorne a 8;
				break;
			}
		}
	} // Do for (;;)
	return true;
}

bool
TeTin::insertSegment( TeCoord2D& pf, TeCoord2D& pn )
{
	if ( this->insertPoint( pf.x(), pf.y(), 100.) )
	{
		if ( this->insertPoint( pn.x(), pn.y(), 100.) )
		{
			std::list<long> aiset;
			std::list<long> viset;
			if ( this->edgesInterBySegment( pf, pn, aiset, viset ) )
			{
				if ( viset.size() == 0 )
				{
					while ( aiset.size() > 0 )
					{
//2.	Para cada aresta ai de A, faÃ§a:
						std::list<long>::iterator aii;
						for (aii = aiset.begin(); aii != aiset.end(); aii++)
						{
							long ai = *aii;
//2.1.	Defina os triÃ¢ngulos td e te compartilham ai,
							long td = edgeSet_[ai].right();
							long te = edgeSet_[ai].left();
//2.2.	Defina os vÃ©rtices vm e vn dos triÃ¢ngulos td e te que nÃ£o estÃ£o sobre a aresta ai,
							long vn = this->vertexOppEdge(td, ai);
							long vm = this->vertexOppEdge(te, ai);
//2.3.	Se r nÃ£o intersecta o segmento que conecta os vÃ©rtices vm e vn, faÃ§a:
							TeCoord2D pi;
							if ( TeSegmentsIntersectPoint(vertexSet_[vn].location(), vertexSet_[vm].location(), pf, pn, pi) )
							{
//2.3.1.	Se ai intersecta o segmento que conecta os vÃ©rtices vm e vn, faÃ§a:
								if ( this->swapEdges( td, te, ai ) )
								{
//2.3.1.1.	Redefina ai como sendo o segmento que conecta os vÃ©rtices vm e vn,
//2.3.1.2.	Redefina adequadamente os triÃ¢ngulos tv e tf,
//2.3.1.3.	Remova a aresta ai de A,
									aiset.remove(ai);
									break;
								}
							}
						}
						if ( aii != aiset.end() )
							continue;
//3.	Se nÃ£o hÃ¡ mais arestas em A, entÃ£o:
//3.1.	Termine o algoritmo.
//4.	Para cada aresta ai de A, faÃ§a:
						aii = aiset.begin();
						while ( aii != aiset.end() )
						{
							long ai = *aii;
//4.1.	Defina os triÃ¢ngulos td e te compartilham ai,
							long td = edgeSet_[ai].right();
							long te = edgeSet_[ai].left();
//4.2.	Defina os vÃ©rtices vm e vn dos triÃ¢ngulos td e te que nÃ£o estÃ£o sobre a aresta ai,
//4.3.	Se ai intersecta o segmento que conecta os vÃ©rtices vm e vn, faÃ§a:
							if ( this->swapEdges( td, te, ai ) )
							{
//4.3.1.	Redefina ai como sendo o segmento que conecta os vÃ©rtices vm e vn,
//4.3.2.	Redefina adequadamente os triÃ¢ngulos tv e tf,
								TeCoord2D pinter;
								long vt = edgeSet_[ai].to();
								long vf = edgeSet_[ai].from();
								TeCoord2D pi;
								if ( ( vertexSet_[vt].location() == pf ) ||
									 ( vertexSet_[vf].location() == pf ) ||
									 ( vertexSet_[vt].location() == pn ) ||
									 ( vertexSet_[vf].location() == pn ) ||
									 ( TeSegmentsIntersectPoint(vertexSet_[vt].location(), vertexSet_[vf].location(), pf, pn, pi) == false ))
								{
//4.3.3.	Se ai nÃ£o intersecta r, faÃ§a:
//4.3.3.1.	Remova a aresta ai de A
									aiset.remove(ai);
									aii = aiset.begin();
								}
								else
									aii++;
							}
							else
								aii++;
						}
//4.3.3.	Retorne a 2.
					}
					return true;
				}
			}
		}
	}
	return false;
}

bool
TeTin::createInitialTriangles(TeBox &box)
{
	// Make box 1% bigger
	zoomOut(box, 1./1.01);

	//	Create and insert initial nodes of the Tin
 	vertexSet_.add( TeTinVertex (box.lowerLeft(), TeMAXFLOAT, ONBOXVERTEX, 0 ) ); //v0
 	vertexSet_.add( TeTinVertex (TeCoord2D(box.x1(),box.y2()), TeMAXFLOAT,
		                         ONBOXVERTEX, 1 ) ); //v1
 	vertexSet_.add( TeTinVertex (box.upperRight(), TeMAXFLOAT, ONBOXVERTEX, 2 ) ); //v2
 	vertexSet_.add( TeTinVertex (TeCoord2D(box.x2(),box.y1()), TeMAXFLOAT,
		                         ONBOXVERTEX, 3 ) ); //v3

//	Create and insert initial lines of the Tin
	edgeSet_.add( TeTinEdge (0, 1, -1, 0, NORMALEDGE) ); //a0
	edgeSet_.add( TeTinEdge (1, 2, -1, 0, NORMALEDGE) ); //a1
	edgeSet_.add( TeTinEdge (2, 3, -1, 1, NORMALEDGE) ); //a2
	edgeSet_.add( TeTinEdge (3, 0, -1, 1, NORMALEDGE) ); //a3
	edgeSet_.add( TeTinEdge (0, 2,  0, 1, NORMALEDGE) ); //a4

//	Create the two initial triangles of the Tin
//	Update triangles edges
	triangleSet_.add ( TeTinTriangle (0, 1, 4) );
	triangleSet_.add ( TeTinTriangle (2, 3, 4) );

	TeBox boxT1, boxT2;
	updateBox(boxT1, vertexSet_[0].location());
	updateBox(boxT1, vertexSet_[1].location());
	updateBox(boxT1, vertexSet_[2].location());
	updateBox(boxT2, vertexSet_[2].location());
	updateBox(boxT2, vertexSet_[3].location());
	updateBox(boxT2, vertexSet_[0].location());

	long indexT = triangleSet_.size() - 2;

	triangleSet_[indexT].setBox(boxT1);
	triangleSet_[indexT+1].setBox(boxT2);

	indexTriang_->insert(boxT1, indexT);
	indexTriang_->insert(boxT2, indexT+1);

	return true;
}

bool
TeTin::insertLineSamples ( TeContourLine& line )
{
	TeLineSimplify(line, 20., 200.);
	unsigned long i;
	for ( i = 0; i < line.size(); i++ )
	{
		TeCoord2D pt = line[i];
		this->insertPoint(pt.x(), pt.y(), line.value());
	}
	return true;
}

bool
TeTin::isEdgeTriangle (long t)
{
	long neighids[3];
	this->triangle3Neighbors(t, neighids[0],neighids[1],neighids[2]);

	long i;
	for (i = 0; i < 3; i++)
		if ( neighids[i] == -1)
			return true;
	return false;
}

void
TeTin::convexize ()
{
	//unsigned long eii;

	//for (eii = 0; eii < edgeSet_.size(); eii++)
	//{
	//	TeTinEdge ei = this->edge(eii);
	//	if ((this->vertex(ei.from()).value() > TeTinBIGFLOAT ) ||
	//		(this->vertex(ei.to  ()).value() > TeTinBIGFLOAT ) )
	//	{
	//		if ((this->vertex(ei.from()).value() > TeTinBIGFLOAT ) && 
	//			(this->vertex(ei.to  ()).value() > TeTinBIGFLOAT ) )
	//			continue;
	//		else
	//		{
	//			long td = ei.right();
	//			long te = ei.left();
	//			if ( ! this->isEdgeTriangle(td) || ! this->isEdgeTriangle(te) )
	//				this->swapEdges( td, te, eii );
	//		}
	//	}
	//}

	long trids[4];
	long nids[4];
	int i;

	for (i = 0; i < 4; i++)
	{
		trids[i] = 0;
		nids[i] = 0;
	}

	i = 0;

	for (long nid = 1; nid < vertexSet_.size(); nid++)
	{
		if ((vertexSet_[nid].type() != ONBOXVERTEX) &&
			(vertexSet_[nid].type() != DELETEDVERTEX))
			continue;
		if (vertexSet_[nid].value() < TeTinBIGFLOAT)
			continue;

		nids[i] = nid;
		i++;

		if (i == 4)
			break;
	}

	bool modified = false;
	for (i = 0; i < 4; i++)
	{
		std::list<long> edges;
		this->vertexEdges(nids[i], edges);

		std::list<long>::iterator itEdge = edges.begin();
		while (itEdge != edges.end())
		{
			long e = *itEdge;
			long rtri = edgeSet_[e].right();
			long ltri = edgeSet_[e].left();

			if ((rtri != -1) && (ltri != -1))
			{
				long neigh[3];
				this->triangle3Neighbors(rtri, neigh[0], neigh[1], neigh[2]);

				int j;
				for (j = 0; j < 3; j++)
				{
					if (neigh[j] == -1)
						break;
				}

				if (j != 3)
				{
					itEdge++;
					continue;
				}

				if (this->swapEdges(rtri, ltri, e))
					modified = true;
			}
			else
			{
				if (((edgeSet_[e].from() == nids[i]) &&
					((edgeSet_[e].to() == nids[(i+1)%4]) ||
					(edgeSet_[e].to() == nids[(i+2)%4]) ||
					(edgeSet_[e].to() == nids[(i+3)%4]))) ||
					((edgeSet_[e].to() == nids[i]) &&
					((edgeSet_[e].from() == nids[(i+1)%4]) ||
					(edgeSet_[e].from() == nids[(i+2)%4]) ||
					(edgeSet_[e].from() == nids[(i+3)%4]))))
				{
					if (rtri == -1)
						trids[i] = ltri;
					else
						trids[i] = rtri;
				}
			}

			itEdge++;
		}

		if ((i == 3) && (modified == true))
		{
			i = -1;
			modified = false;
		}
	}

	for (i = 0; i < 4; i++)
	{
		long es[3];
		triangleSet_[trids[i]].edges(es[0], es[1], es[2]);

		for (int j = 0; j < 3; j++)
		{
			long rtri = edgeSet_[es[j]].right();
			long ltri = edgeSet_[es[j]].left();

			if ((rtri != -1) && (ltri != -1))
				this->swapEdges(rtri, ltri, es[j]);
		}
	}
}

//By Fernanda and Eduilson
bool
TeTin::createTIN(TeSampleSet& sampleSet, TeContourLineSet& contourSet)
{
	if ((sampleSet.size() == 0) && (contourSet.size() == 0))
	{
		errorMessage_ = "There is no sample to create TIN.";
		return false;
	}

	//Creates original TIN
	TeBox tinBox;

	if (sampleSet.size() == 0)
		tinBox = TeBox(contourSet.box());
	else if (contourSet.size() == 0)
		tinBox = TeBox(sampleSet.box());
	else
	{
		updateBox(tinBox, sampleSet.box());
		updateBox(tinBox, contourSet.box());
	}

	this->createInitialTriangles(tinBox);

	//Sets precision used for building TIN
	double tol = TePrecision::instance().precision(); // Save old precision
	double tinPrecision = tinBox.width() / 1.e6; // 1.e6th of deltaX
	TePrecision::instance().setPrecision(tinPrecision);

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("Create TIN");
		TeProgress::instance()->setMessage("Creating TIN...");
		TeProgress::instance()->setTotalSteps(sampleSet.size() + contourSet.size());
	}

	TeLine2D lineAux;
	int step = 0;

	//Insert samples in TIN
	verttype_ = NORMALVERTEX;

	TeSampleSet::iterator si;
	for (si = sampleSet.begin(); si != sampleSet.end(); si++)
	{
		step++;

		double x = (*si).location().x();
		double y = (*si).location().y();
		double z = (*si).value();
		this->insertPoint(x, y, z);

		lineAux.add(TeCoord2D(x, y));

		TeProgress::instance()->setProgress(step);

		if (TeProgress::instance()->wasCancelled())
			return false;
	}

	if (isoTol_ == -1)
		isoTol_ = tinPrecision*400.;

	if (ptsdist_ == -1)
		ptsdist_ = tinPrecision*2000;

	//verttype_ = ONCONTOURVERTEX;

	TeContourLineSet::iterator ci;
	for (ci = contourSet.begin(); ci != contourSet.end(); ci++)
	{
		step++;

		TeContourLine line( *ci );

		TeLineSimplify(line, isoTol_, ptsdist_);

		if (line.size() < 2)
			continue;

		short nflag = 1;

		double z = (*ci).value();

		TeContourLine::iterator li;
		for (li = line.begin(); li != line.end(); li++)
		{
			if (!lineAux.empty())
			{
				if (TeWithin(*li, lineAux))
					continue;
			}

			double x = (*li).x();
			double y = (*li).y();

			//Set vertex type
			if (nflag)
				verttype_ = CONTOURFIRSTVERTEX;
			else
				verttype_ = ONCONTOURVERTEX;

			this->insertPoint(x, y, z);

			if (nflag)
			{
				int v = vertexSet_.size() - 1;

				if (vertexSet_[v].type() == DELETEDVERTEX)
					nflag = 1;
				else
					nflag = 0;
			}

			lineAux.add(*li);
		}

		int v = vertexSet_.size() - 1;
		if (vertexSet_[v].type() == ONCONTOURVERTEX)
			vertexSet_[v].setType(CONTOURLASTVERTEX);
		else if (vertexSet_[v].type() == CONTOURFIRSTVERTEX)
			vertexSet_[v].setType(NORMALVERTEX);

		TeProgress::instance()->setProgress(step);

		if (TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			return false;
		}
	}

	//Fixes edge triangles TIN
	this->convexize();

	//Remove ONBOXVERTEX and DELETEDVERTEX
	this->borderUp();

	TePrecision::instance().setPrecision(tol);

	TeProgress::instance()->reset();

	return true;
}

bool 
TeTin::createTIN(TeLayer* inputLayer_ptr, TePolygonSet &ps)
{
  TePolygon tp;
  TeLinearRing tl;
  int id = 0;
  int j = 0;
  int edgeit = 0;

//Set querier parameters
  bool loadGeometries = true;
  bool loadAllAttributes = true;
  TeQuerierParams querierParams(loadGeometries, loadAllAttributes);
  querierParams.setParams(inputLayer_ptr);
  TeQuerier querier(querierParams);

//Load instances from layer based in querier parameters
  if(querier.loadInstances())
  {
//  Returns loaded attributes list 
	TeAttributeList attrList = querier.getAttrList();
  }

  //! Used to store the attribute's values
  vector<string> attrValue (0);
  //! Used to store the attribute's objects ID
  vector<string> attrId (0);
  int it = 0;
  int it_id = 0;
  int vecSize = 0;
  int vecIdSize = 0;
  int I = 0;
  string Id;
  //! Containers used to store the attributes values and objects ID
  map<string, string> val1;
  map<string, string> val2;
  map<string, string> val3;

//Traverse all the instances
  TeSTInstance sti;
  while(querier.fetchInstance(sti))
  {// Get attribute's values
    TePropertyVector vec = sti.getPropertyVector();
	it_id+=1;
	vecIdSize = it_id;
	attrId.resize(vecIdSize);
	attrId[I] = sti.getObjectId();
	Id = attrId[I];
	for(unsigned int i=0; i<vec.size(); ++i)
	{
	  string attrName = vec[i].attr_.rep_.name_;
	  if(attrName == "val1")
	  {
	    it+=1;
		vecSize = it;
		attrValue.resize(vecSize);
		attrValue[j] = vec[i].value_;
		val1[Id] = attrValue[j];
		j+=1;
	  }
	  if(attrName == "val2")
	  {
	    it+=1;
		vecSize = it;
		attrValue.resize(vecSize);
		attrValue[j] = vec[i].value_;
		val2[Id] = attrValue[j];
		j+=1;
	  }
	  if(attrName == "val3")
	  {
		it+=1;
		vecSize = it;
		attrValue.resize(vecSize);
		attrValue[j] = vec[i].value_;
		val3[Id] = attrValue[j];
		j+=1;
	  }
    }
	I+=1;
  }

  TeBox tinBox(ps.box());
  
  for (unsigned int i=0; i<ps.size(); ++i)
  {
    tp=ps[i];
	string psId = tp.objectId();
	string value1 = val1[psId];
	string value2 = val2[psId];
	string value3 = val3[psId];
	tl=tp.first();

//  Get 3 polygon's points
	TeSample sample1 (TeCoord2D(tl[0].x(), tl[0].y()), atof(value1.c_str()));
	TeSample sample2 (TeCoord2D(tl[1].x(), tl[1].y()), atof(value2.c_str()));
	TeSample sample3 (TeCoord2D(tl[2].x(), tl[2].y()), atof(value3.c_str()));
//  Creates samples object ID
	sample1.objectId(Te2String(id));
	sample2.objectId(Te2String(id+1));
	sample3.objectId(Te2String(id+2));
	TeCoord2D s1 = sample1.location();
	double v1 = sample1.value();
	TeCoord2D s2 = sample2.location();
	double v2 = sample2.value();
	TeCoord2D s3 = sample3.location();
	double v3 = sample3.value();

//  Insert TIN nodes
	TeTinVertexSet::iterator vi = vertexSet_.begin();
	//if (!vertexSet_.find( TeTinVertex(s1,v1,NORMALVERTEX,0),vi) )
	//{
	//  vertexSet_.add( TeTinVertex(s1, v1, NORMALVERTEX, 0));
	//  vi=vertexSet_.end()-1;
	//  vi->objectId(sample1.objectId());
	//}
//  Gets vertex id
	string id1 = vi->objectId();
	//if(!vertexSet_.find( TeTinVertex(s2,v2,NORMALVERTEX,0),vi) )
	//{
	//  vertexSet_.add( TeTinVertex(s2, v2, NORMALVERTEX, 0));
	//  vi=vertexSet_.end()-1;
	//  vi->objectId(sample2.objectId());
	//}
	string id2 = vi->objectId();
	//if(!vertexSet_.find( TeTinVertex(s3,v3,NORMALVERTEX,0),vi) )
	//{
	//  vertexSet_.add( TeTinVertex(s3, v3, NORMALVERTEX, 0));
	//  vi=vertexSet_.end()-1;
	//  vi->objectId(sample3.objectId());
	//}
	string id3 = vi->objectId();

//  Insert TIN lines
	TeTinEdgeSet::iterator ei = edgeSet_.begin();
	//if (!edgeSet_.find( TeTinEdge(atol(id1.c_str()),atol(id2.c_str()),-1,-1,
	//	                          NORMALEDGE),ei) )
	//{
	//  edgeSet_.add( TeTinEdge(atol(id1.c_str()),atol(id2.c_str()),-1,-1,
	//	                      NORMALEDGE) );
	//  ei=edgeSet_.end()-1;
	//  ei->objectId(Te2String(edgeit));
	//  ++edgeit;
	//}
//  Gets the edge id
	string idedge1 = ei->objectId();
	//if (!edgeSet_.find( TeTinEdge(atol(id2.c_str()),atol(id3.c_str()),-1,-1,
	//	                          NORMALEDGE),ei) )
	//{
	//  edgeSet_.add( TeTinEdge(atol(id2.c_str()),atol(id3.c_str()),-1,-1,
	//	                      NORMALEDGE) );
	//  ei=edgeSet_.end()-1;
	//  ei->objectId(Te2String(edgeit));
	//  ++edgeit;
	//}
	string idedge2 = ei->objectId();
	//if (!edgeSet_.find( TeTinEdge(atol(id3.c_str()),atol(id1.c_str()),-1,-1,
	//	                          NORMALEDGE),ei) )
	//{
	//  edgeSet_.add( TeTinEdge(atol(id3.c_str()),atol(id1.c_str()),-1,-1,
	//	                      NORMALEDGE) );
	//  ei=edgeSet_.end()-1;
	//  ei->objectId(Te2String(edgeit));
	//  ++edgeit;
	//}
	string idedge3 = ei->objectId();

//  Insert TIN triangles
	triangleSet_.add( TeTinTriangle(atol(idedge1.c_str()),atol(idedge2.c_str()),
		                            atol(idedge3.c_str())) );
	id = id + 3;
  }

  return true;
}

bool
TeTin::createTIN(TeLayer* inputLayer_ptr)
{
	//Set querier parameters
	bool loadGeometries = true;
	bool loadAllAttributes = true;
	TeQuerierParams querierParams(loadGeometries, loadAllAttributes);
	querierParams.setParams(inputLayer_ptr);
	TeQuerier querier(querierParams);
	TeBox tinBox(querierParams.box());

	//Load instances from layer based in querier parameters
	if(!querier.loadInstances())
	{
		cout << "Erro no carregamento dos objetos do layer" << endl;
	}
	
	// Traverse all the instances 
	TeSTInstance sti;
	int i=0;
	TeSampleSet sampleSet;
	double x; 
	double y; 
	double z;
	while(querier.fetchInstance(sti))
	{
		// Plot each attribute, its name and value
		TePropertyVector vec = sti.getPropertyVector();
		for(unsigned int i=0; i<vec.size(); ++i)
		{
			string  attrName = vec[i].attr_.rep_.name_;
			if (attrName =="z")
			{
				string  attrValue = vec[i].value_;
				z = atof (attrValue.c_str());
			}
		}

		//Get geometries
		if(sti.hasPoints())
		{
			TePointSet ponSet;
			TePoint    pt;
			sti.getGeometry(ponSet);

			for(unsigned int i=0; i<ponSet.size(); ++i)
			{
				x = atof ( Te2String(ponSet[i].location().x(), 7).c_str() );
				y = atof ( Te2String(ponSet[i].location().y(), 7).c_str());
			}
		}

		cout << endl << endl;
		//discard ONBOXVERTEX's vertex
		if (z <= 340000000)
			sampleSet.add ( TeSample(TeCoord2D(x, y), z) );

	}
	TeContourLineSet contourSet;
	this->createTIN(sampleSet,contourSet);

  return true;
}

bool
TeTin::createAttrTable(TeTable& attrTable, TeDatabase* db, TeLayer* layer)
{
/*
//Creates an attribute table
//Defines a list of attributes
  TeAttributeList attList;
  TeAttribute at;
  at.rep_.type_ = TeSTRING;
  at.rep_.numChar_ = 16;
  at.rep_.name_ = "triangle_id";
  at.rep_.isPrimaryKey_ = true;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "val1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "val2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "val3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "x1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "y1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "x2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "y2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "x3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "y3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "slope";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "aspect";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  string tablename (attrTable.name());
//Creates an attribute table associated to the new layer
//Creates a table in memory
  TeTable attTable(tablename, attList, "triangle_id", "triangle_id");
  if (!layer->createAttributeTable(attTable))
	 return false;

  attrTable = attTable;
  return true;*/

	//Creates an attribute table
//Defines a list of attributes
  TeAttributeList attList;
  TeAttribute at;
  at.rep_.type_ = TeSTRING;
  at.rep_.numChar_ = 16;
  at.rep_.name_ = "linkcol";
  at.rep_.isPrimaryKey_ = true;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "triangle_id";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "val1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "val2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeREAL;
  at.rep_.decimals_ = 6;
  at.rep_.name_ = "val3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeSTRING;
  at.rep_.numChar_ = 25;
  at.rep_.name_ = "type1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeSTRING;
  at.rep_.numChar_ = 25;
  at.rep_.name_ = "type2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeSTRING;
  at.rep_.numChar_ = 25;
  at.rep_.name_ = "type3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "left1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "right1";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "left2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "right2";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "left3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  at.rep_.type_ = TeINT;
  at.rep_.name_ = "right3";
  at.rep_.isPrimaryKey_ = false;
  attList.push_back(at);

  string tablename (attrTable.name());
//Creates an attribute table associated to the new layer
//Creates a table in memory
  TeTable attTable(tablename, attList, "linkcol", "linkcol");
  if (!layer->createAttributeTable(attTable))
	 return false;

  attrTable = attTable;
  return true;
}

bool
TeTin::createPolygon(TePolygonSet ps, TeTable& attTable, int t, TeCoord2D& pt0,
			         TeCoord2D& pt1, TeCoord2D& pt2, double& val0, double& val1,
					 double& val2)
{
//Creates a set of polygons
  TeLine2D line;
  line.add(pt0);
  line.add(pt1);
  line.add(pt2);
  line.add(pt0);

  TeLinearRing r1(line);
  TePolygon poly1;
  poly1.add(r1);

  poly1.objectId(Te2String(t));

  ps.add(poly1);

//Each row is related to one of the geometries by its objetc_id
  TeTableRow row;
  row.push_back(Te2String(t));
  row.push_back(Te2String(val0,6));
  row.push_back(Te2String(val1,6));
  row.push_back(Te2String(val2,6));
//Adds the row in the attributes table
  attTable.add(row);
  row.clear();

  return true;
}

bool
TeTin::createPolygon(TePolygonSet ps, TeTable& attTable, int t, TeCoord2D& pt0,
			         TeCoord2D& pt1, TeCoord2D& pt2, double& val0, double& val1,
					 double& val2,double& slope, double& aspect)
{
//Creates a set of polygons
  TeLine2D line;
  line.add(pt0);
  line.add(pt1);
  line.add(pt2);
  line.add(pt0);

  TeLinearRing r1(line);
  TePolygon poly1;
  poly1.add(r1);

  poly1.objectId(Te2String(t));

  ps.add(poly1);

//Each row is related to one of the geometries by its objetc_id
  TeTableRow row;
  row.push_back(Te2String(t));
  row.push_back(Te2String(val0,6));
  row.push_back(Te2String(val1,6));
  row.push_back(Te2String(val2,6));
  row.push_back(Te2String(pt0.x(),6));
  row.push_back(Te2String(pt0.y(),6));
  row.push_back(Te2String(pt1.x(),6));
  row.push_back(Te2String(pt1.y(),6));
  row.push_back(Te2String(pt2.x(),6));
  row.push_back(Te2String(pt2.y(),6));
  row.push_back(Te2String(slope,2));
  row.push_back(Te2String(aspect,2));
  
//Adds the row in the attributes table
  attTable.add(row);
  row.clear();

  return true;
}

bool
TeTin::createPolygon(TePolygonSet ps, TeTable& attTable, int t, TeTinVertex vert0,
			         TeTinVertex vert1, TeTinVertex vert2, double& slope, double& aspect)
{
  TeTinTriangle triang = this->triangle(t);

  long edge0, edge1, edge2;
  triang.edges(edge0, edge1, edge2);

  long left1 = this->edge(edge0).left();
  long right1 = this->edge(edge0).right();
  long left2 = this->edge(edge1).left();
  long right2 = this->edge(edge1).right();
  long left3 = this->edge(edge2).left();
  long right3 = this->edge(edge2).right();

  //if (vert0.type() != ONBOXVERTEX && vert1.type() != ONBOXVERTEX && vert2.type() != ONBOXVERTEX)
  //{
	  TeCoord2D pt0, pt1, pt2;
	  pt0 = vert0.location();
	  pt1 = vert1.location();
	  pt2 = vert2.location();

	//Creates a set of polygons
	  TeLine2D line;
	  line.add(pt0);
	  line.add(pt1);
	  line.add(pt2);
	  line.add(pt0);

	  TeLinearRing r1(line);
	  TePolygon poly1;
	  poly1.add(r1);

	  poly1.objectId(Te2String(t));

	  ps.add(poly1);
  //}

/*
  long e[3];
  if (this->edge(edge0).left() != t)
  {
	  if (left1 != -1)
	  {
		  triangleSet_[left1].edges(e[0], e[1], e[2]);

		  if ((this->edge(e[0]).type() == DELETEDEDGE) || (this->edge(e[1]).type() == DELETEDEDGE) || (this->edge(e[2]).type() == DELETEDEDGE))
		  {
			  left1 = -1;
			  this->edge(edge0).setTriangles(left1, right1);
		  }
	  }
  }
  else
  {
	  if (right1 != -1)
	  {
		  triangleSet_[right1].edges(e[0], e[1], e[2]);

		  if ((this->edge(e[0]).type() == DELETEDEDGE) || (this->edge(e[1]).type() == DELETEDEDGE) || (this->edge(e[2]).type() == DELETEDEDGE))
		  {
			  right1 = -1;
			  this->edge(edge0).setTriangles(left1, right1);
		  }
	  }
  }

  if (this->edge(edge1).left() != t)
  {
	  if (left2 != -1)
	  {
		  triangleSet_[left2].edges(e[0], e[1], e[2]);

		  if ((this->edge(e[0]).type() == DELETEDEDGE) || (this->edge(e[1]).type() == DELETEDEDGE) || (this->edge(e[2]).type() == DELETEDEDGE))
		  {
			  left2 = -1;
			  this->edge(edge1).setTriangles(left2, right2);
		  }
	  }
  }
  else
  {
	  if (right2 != -1)
	  {
		  triangleSet_[right2].edges(e[0], e[1], e[2]);

		  if ((this->edge(e[0]).type() == DELETEDEDGE) || (this->edge(e[1]).type() == DELETEDEDGE) || (this->edge(e[2]).type() == DELETEDEDGE))
		  {
			  right2 = -1;
			  this->edge(edge1).setTriangles(left2, right2);
		  }
	  }
  }

  if (this->edge(edge2).left() != t)
  {
	  if (left3 != -1)
	  {
		  triangleSet_[left3].edges(e[0], e[1], e[2]);

		  if ((this->edge(e[0]).type() == DELETEDEDGE) || (this->edge(e[1]).type() == DELETEDEDGE) || (this->edge(e[2]).type() == DELETEDEDGE))
		  {
			  left3 = -1;
			  this->edge(edge2).setTriangles(left3, right3);
		  }
	  }
  }
  else
  {
	  if (right3 != -1)
	  {
		  triangleSet_[right3].edges(e[0], e[1], e[2]);

		  if ((this->edge(e[0]).type() == DELETEDEDGE) || (this->edge(e[1]).type() == DELETEDEDGE) || (this->edge(e[2]).type() == DELETEDEDGE))
		  {
			  right3 = -1;
			  this->edge(edge2).setTriangles(left3, right3);
		  }
	  }
  }
  */

//Each row is related to one of the geometries by its objetc_id
  TeTableRow row;
  row.push_back(Te2String(t));
  row.push_back(Te2String(t));
  row.push_back(Te2String(vert0.value(),6));
  row.push_back(Te2String(vert1.value(),6));
  row.push_back(Te2String(vert2.value(),6));
  row.push_back(Te2String(vert0.type()));
  row.push_back(Te2String(vert1.type()));
  row.push_back(Te2String(vert2.type()));
  row.push_back(Te2String(left1));
  row.push_back(Te2String(right1));
  row.push_back(Te2String(left2));
  row.push_back(Te2String(right2));
  row.push_back(Te2String(left3));
  row.push_back(Te2String(right3));
  
//Adds the row in the attributes table
  attTable.add(row);
  row.clear();

  return true;
}

bool
TeTin::saveTIN(TeDatabase* db, TeLayer* layer, TePolygonSet& ps,
			   TeTable& attTable)
{
//Adds the polygon set to the layer
//As addGeometry wasnÂ´t called before, the table of polygons will have a default name
  if (!layer->addPolygons(ps))
	  return false;

//Saves attributes table in database
  if (!layer->saveAttributeTable(attTable))
	  return false;

	return true;
}

bool
TeTin::saveVertexTIN(TeDatabase* db, TeLayer* layer, TePointSet& ps,
			   TeTable& attTable)
{
//Adds the point set to the layer
//As addGeometry wasnÂ´t called before, the table of polygons will have a default name
	if (!layer->addPoints(ps))
	  return false;

//Saves attributes table in database
  if (!layer->saveAttributeTable(attTable))
	  return false;

  return true;
}


bool
TeTin::saveTINdb(TeDatabase* db, string& outlayername, string& tablename)
{
  int triang = 0;
  TePolygonSet ps;
  TeBox bbox = this->box();

//Creates a projection
  TeDatum mDatum = TeDatumFactory::make("SAD69");
  TeProjection* pUTM = new TeUtm(mDatum,0.0);

//Checks if the attributes table already exist in database
  if(db->tableExist(tablename))
	  return false;

//Checks if the layer already exist in database
  if (db->layerExist(outlayername))
	  return false;

//Creates a new layer with selected layer's name
  TeLayer* layer = new TeLayer(outlayername, db, bbox, pUTM);
  if (layer->id() <= 0)   // layer wasnÂ´t created properly
	  return false;

//Adds the geometry representation Polygons and stores it in a table with default name
  if (!layer->addGeometry(TePOLYGONS))
	  return false;

//Creates attribute table with name selected by user
  TeTable attTable (tablename);
  if(!createAttrTable(attTable, db, layer))
	  return false;

  TeTinTriangleIterator ti;
  for (ti = this->triangleBegin(); ti != this->triangleEnd(); ti++)
  {
	  long v0,v1,v2;
	  this->triangleVertices(*ti,v0,v1,v2);
	  if( vertex(v0).type() != ONBOXVERTEX && vertex(v1).type() != ONBOXVERTEX && vertex(v2).type() != ONBOXVERTEX) 
	  {
          TeCoord2D pt[3];
		  double val[3];

		//  Create triangles points
		  this->triangle3DPoints(*ti, pt[0], pt[1], pt[2], val[0], val[1], val[2]);

		//	Calculates triangle's slope
		  double slope;
		  this->slopeTriangle(*ti,slope,'g');

		// Calculates triangle's aspect
		  double aspect;
		  this->aspectTriangle(*ti,aspect);

		//  Create triangles
		  if (!createPolygon(ps, attTable, triang, pt[0], pt[1], pt[2], val[0],
						val[1], val[2],slope,aspect))
		  return false;

		  triang++;
	  }
  }
 
  if (!saveTIN(db, layer, ps, attTable))
	  return false;
  
  return true;
}

TeLayer*
TeTin::saveTINdb(TeDatabase* db, TeProjection* proj, string& outlayername, string& tablename)
{
  int triang = 0;
  TePolygonSet ps;
  TeBox bbox = this->box();

  if (TeProgress::instance())
  {
	  TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
	  TeProgress::instance()->setMessage("Saving TIN in database...");
	  TeProgress::instance()->setTotalSteps(triangleSet_.size());
  }

//Creates a projection
  TeProjection* pUTM = proj;

//Checks if the attributes table already exist in database
  if(db->tableExist(tablename))
	  return NULL;

//Checks if the layer already exist in database
  if (db->layerExist(outlayername))
	  return NULL;

//Creates a new layer with selected layer's name
  TeLayer* layer = new TeLayer(outlayername, db, bbox, pUTM);
  if (layer->id() <= 0)   // layer was not created properly
	  return NULL;

//Adds the geometry representation Polygons and stores it in a table with default name
  if (!layer->addGeometry(TePOLYGONS))
	  return NULL;

//Creates attribute table with name selected by user
  TeTable attTable (tablename);
  if(!createAttrTable(attTable, db, layer))
	  return NULL;

  triang = 0;
  TeTinTriangleIterator ti;
  for (ti = this->triangleBegin(); ti != this->triangleEnd(); ti++)
  {
	  if (TeProgress::instance())
	  {
		  TeProgress::instance()->setProgress(triang);
	  }

	  if (TeProgress::instance()->wasCancelled())
	  {
		  TeProgress::instance()->reset();
		  errorMessage_ = "Operation was cancelled.";
		  delete layer;
		  return NULL;
	  }

	  long v0,v1,v2;
	  this->triangleVertices(*ti,v0,v1,v2);
	  //if( vertex(v0).type() != ONBOXVERTEX && vertex(v1).type() != ONBOXVERTEX && vertex(v2).type() != ONBOXVERTEX) 
	  //{
          //TeCoord2D pt[3];
		  //double val[3];

		//  Create triangles points
		  //this->triangle3DPoints(*ti, pt[0], pt[1], pt[2], val[0], val[1], val[2]);

		//	Calculates triangle's slope
		  double slope;
		  this->slopeTriangle(*ti,slope,'g');

		// Calculates triangle's aspect
		  double aspect;
		  this->aspectTriangle(*ti,aspect);

		//  Create triangles
		  //if (!createPolygon(ps, attTable, triang, pt[0], pt[1], pt[2], val[0],
				//		val[1], val[2],slope,aspect))
		  if (!createPolygon(ps, attTable, triang, vertex(v0), vertex(v1), vertex(v2), slope,aspect))
			return NULL;
	  //}

	  triang++;
  }

  if (TeProgress::instance())
	  TeProgress::instance()->reset();

  if (ps.empty())
  {
	  errorMessage_ = "Error to create TIN. No triangle was created.";
	  delete layer;
	  return NULL;
  }
 
  if (!saveTIN(db, layer, ps, attTable))
  {
	  delete layer;
	  return NULL;
  }
  
  return layer;
}

bool 
TeTin::saveVertexTINdb(TeDatabase* db, string& outlayername, string& tablename)
{
  TePointSet ps;
  TeBox bbox = this->box();

//Creates a projection
  TeDatum mDatum = TeDatumFactory::make("SAD69");
  TeProjection* pUTM = new TeUtm(mDatum,0.0);

//Checks if the attributes table already exist in database
  if(db->tableExist(tablename))
	  return false;

//Checks if the layer already exist in database
  if (db->layerExist(outlayername))
	  return false;

//Creates a new layer with selected layer's name
  TeLayer* layer = new TeLayer(outlayername, db, bbox, pUTM);
  if (layer->id() <= 0)   // layer wasnÂ´t created properly
	  return false;

//Adds the geometry representation Points and stores it in a table with default name
  if (!layer->addGeometry(TePOINTS))
	  return false;

//Creates attribute table with name selected by user
  TeTable attTable (tablename);

//Creates PointSet with TIN's vertex
  if(!this->createPointSet(ps,attTable,layer,db))
		return false;
   
  if (!saveVertexTIN(db, layer, ps, attTable))
	  return false;
  
  return true;
}

bool
TeTin::borderUp ()
{
  long vii;
  long eii;
  long tit;
  long edge0, edge1, edge2;
  TeTinVertex vv;
  TeTinEdge e;
  TeTinTriangle t;

  int nsteps = vertexSet_.size() + edgeSet_.size() + triangleSet_.size();
  if (TeProgress::instance())
  {
	  TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
	  TeProgress::instance()->setMessage("Updating triangulation...");
	  TeProgress::instance()->setTotalSteps(nsteps);
  }

  int step = 0;

  //Search ONBOXVERTEX vertex
  for (vii = 0; vii < vertexSet_.size(); ++vii)
  {
	  TeProgress::instance()->setProgress(step);

	  if ((vertexSet_[vii].type() == ONBOXVERTEX) || (vertexSet_[vii].type() == DELETEDVERTEX))
	  {//if vertex has ONBOXVERTEX type
	    //Search adjacent edges to the vertex
	    std::list <long> vauxedges;
		vertexEdges(vii, vauxedges);

		std::list<long>::iterator eit;
		for (eit = vauxedges.begin(); eit != vauxedges.end(); ++eit)
		{
		  //Swaps vertex reference in the edges (from and/or to) for NULL
		  long from = edgeSet_[(*eit)].from();
		  long to = edgeSet_[(*eit)].to();

		  if (from == vii)
			  edgeSet_[(*eit)].setVertices(-1, to);
		  if (to == vii)
			  edgeSet_[(*eit)].setVertices(from, -1);
		}
		
		long viiaux;
		//For all vertex with index bigger than removed vertex
		for (viiaux = vii + 1; viiaux < vertexSet_.size(); ++viiaux)
		{
			std::list <long> vedges;
		    this->vertexEdges(viiaux, vedges);

		    //Updates vertex index in the adjacent edges
		    for (eit = vedges.begin(); eit != vedges.end(); ++eit)
		    {
		      long from = edgeSet_[(*eit)].from();
		      long to = edgeSet_[(*eit)].to();

			  if (from == viiaux)
                  edgeSet_[(*eit)].setVertices(from-1, to);
			  if (to == viiaux)
                  edgeSet_[(*eit)].setVertices(from, to-1);
		    }
		}

		//Removes vertex from vertexSet
		vertexSet_.erase(vertexSet_[vii]);
		vii = vii - 1;
	  }//end if vertex has ONBOXVERTEX type

	  if (TeProgress::instance()->wasCancelled())
	  {
		  errorMessage_ = "Operation was cancelled.";
		  TeProgress::instance()->reset();
		  return false;
	  }

	  step++;
  }

  //Search edges with NULL vertex from and/or to
  for (eii = 0; eii < edgeSet_.size(); ++eii)
  {
	TeProgress::instance()->setProgress(step);

	long vfrom = edgeSet_[eii].from();
	long vto = edgeSet_[eii].to();

	if (vfrom == -1 || vto == -1)
	{//if from or to are NULL
	//Swaps edge reference in the triangles for NULL
	  long tleft = edgeSet_[eii].left();
	  long tright = edgeSet_[eii].right();

	  //Check if triangle edges are equal eii
	  if (tleft != -1)
	  {
	    //left triangle
	    triangleSet_[tleft].edges(edge0, edge1, edge2);
	    if (edge0 == eii)
		    triangleSet_[tleft].setEdges(-1, edge1, edge2);
	    if (edge1 == eii)
		    triangleSet_[tleft].setEdges(edge0, -1, edge2);
	    if (edge2 == eii)
		    triangleSet_[tleft].setEdges(edge0, edge1, -1);
	  }
	  if (tright != -1)
	  {
	    //right triangle
	    triangleSet_[tright].edges(edge0, edge1, edge2);
	    if (edge0 == eii)
		    triangleSet_[tright].setEdges(-1, edge1, edge2);
	    if (edge1 == eii)
		    triangleSet_[tright].setEdges(edge0, -1, edge2);
	    if (edge2 == eii)
		    triangleSet_[tright].setEdges(edge0, edge1, -1);
	  }

	  //Checks if vertex from and to have reference to this edge in onEdge
	  //If, there is the reference, changes it to another adjacent edge.
	  std::list <long> edgesadj;
	  std::list<long>::iterator eit = edgesadj.begin();
	  if (vfrom != -1)
	  {
	    if (vertexSet_[vfrom].oneEdge() == eii)//vertex from
	    {
	      this->vertexEdges(vfrom, edgesadj);
		  for (eit = edgesadj.begin(); eit != edgesadj.end(); ++eit)
		  {
		    if ((*eit) != -1 && (*eit) != eii)
		    {
		      vertexSet_[vfrom].setOneEdge((*eit));
		      break;
		    }
		  }
	    }
	  }

	  if (vto != -1)
	  {
	    if (vertexSet_[vto].oneEdge() == eii)//vertex to
	    {
	      this->vertexEdges(vto, edgesadj);
		  for (eit = edgesadj.begin(); eit != edgesadj.end(); ++eit)
		  {
		    if ((*eit) != -1 && (*eit) != eii)
		    {
		      vertexSet_[vto].setOneEdge((*eit));
			  break;
		    }
		  }
	    }
	  }
	  
	  long eiiaux;
	  for (eiiaux = eii + 1; eiiaux < edgeSet_.size(); ++eiiaux)
	  {//For all edges with index bigger than removed edge
	    //Updates edge index in the triangles left and right
		  long left = edgeSet_[eiiaux].left();
		  long right = edgeSet_[eiiaux].right();

		  if (left != -1)
	      {
	        //left triangle
	        triangleSet_[left].edges(edge0, edge1, edge2);
	        if (edge0 == eiiaux)
		        triangleSet_[left].setEdges(edge0 - 1, edge1, edge2);
	        if (edge1 == eiiaux)
		        triangleSet_[left].setEdges(edge0, edge1 - 1, edge2);
	        if (edge2 == eiiaux)
		        triangleSet_[left].setEdges(edge0, edge1, edge2 - 1);
	      }
	      if (right != -1)
	      {
            //right triangle
	        triangleSet_[right].edges(edge0, edge1, edge2);
	        if (edge0 == eiiaux)
		        triangleSet_[right].setEdges(edge0 - 1, edge1, edge2);
	        if (edge1 == eiiaux)
		        triangleSet_[right].setEdges(edge0, edge1 - 1, edge2);
	        if (edge2 == eiiaux)
		        triangleSet_[right].setEdges(edge0, edge1, edge2 - 1);
	      }

	    //Updates onEdge of vertex
		  if (edgeSet_[eiiaux].from() != -1)
		  {
			if (vertexSet_[edgeSet_[eiiaux].from()].oneEdge() >= eiiaux)
		  		vertexSet_[edgeSet_[eiiaux].from()].setOneEdge(eiiaux-1);
		  }
		  if (edgeSet_[eiiaux].to() != -1)
		  {
			if (vertexSet_[edgeSet_[eiiaux].to()].oneEdge() >= eiiaux)
		  		vertexSet_[edgeSet_[eiiaux].to()].setOneEdge(eiiaux-1);
		  }
	  }

	  //Removes edge from edgeSet
	  edgeSet_.erase(edgeSet_[eii]);
	  eii = eii - 1;
	}//end if from or to are NULL

	if (TeProgress::instance()->wasCancelled())
	{
		errorMessage_ = "Operation was cancelled.";
		TeProgress::instance()->reset();
		return false;
	}

	step++;
  }

  indexTriang_->clear();

  //Search triangles with NULL reference
  for (tit = 0; tit < triangleSet_.size(); ++tit)
  {
	TeProgress::instance()->setProgress(step);

	triangleSet_[tit].edges(edge0, edge1, edge2);
	if (edge0 == -1 || edge1 == -1 || edge2 == -1)
	{//if triangle has NULL edge
	  //For each triangle to be removed:
	  //Changes triangle reference in the edges (left and right) to -1
	  if (edge0 != -1)
	  {
		long left = edgeSet_[edge0].left();
		long right = edgeSet_[edge0].right();

		if (left == tit)
		    edgeSet_[edge0].setTriangles(-1, right);
		if (right == tit)
		    edgeSet_[edge0].setTriangles(left, -1);
      }

	  if (edge1 != -1)
	  {
		long left = edgeSet_[edge1].left();
		long right = edgeSet_[edge1].right();

	    if (left == tit)
		    edgeSet_[edge1].setTriangles(-1, right);
		if (right == tit)
		    edgeSet_[edge1].setTriangles(left, -1);
	  }
	  if (edge2 != -1)
	  {
		long left = edgeSet_[edge2].left();
		long right = edgeSet_[edge2].right();

	    if (left == tit)
		    edgeSet_[edge2].setTriangles(-1, right);
		if (right == tit)
		    edgeSet_[edge2].setTriangles(left, -1);
	  }

	  //Updates triangle left and/or right of edges
	  for (eii = 0; eii < edgeSet_.size(); ++eii)
	  {
          //Swaps triangles left and/or right reference in the edges for 
          long tleft = edgeSet_[eii].left();
          long tright = edgeSet_[eii].right();

		  if (tleft >= tit)
			  tleft = tleft - 1;
		  if (tright >= tit)
			  tright = tright - 1;

		  edgeSet_[eii].setTriangles(tleft, tright);
	  }

	  //Removes triangle from triangleSet
	  triangleSet_.erase(triangleSet_[tit]);
	  tit = tit - 1;
	}//end if triangle has NULL edge

	if (TeProgress::instance()->wasCancelled())
	{
		errorMessage_ = "Operation was cancelled.";
		TeProgress::instance()->reset();
		return false;
	}

	step++;
  }

  TeCoord2D pt0, pt1, pt2;

  for (tit = 0; tit < triangleSet_.size(); ++tit)
  {
	this->trianglePoints(tit, pt0, pt1, pt2);

	TeBox boxT;
	updateBox(boxT, pt0);
	updateBox(boxT, pt1);
	updateBox(boxT, pt2);

	triangleSet_[tit].setBox(boxT);
	indexTriang_->insert(boxT, tit);
  }

  TeProgress::instance()->reset();

  return true;
}

void
TeTin::writeTriangleEdgesSPRFile (std::string& name, TeRaster* raster)
{
	fstream sprcfile;
	sprcfile.open ( name.c_str(), ios::out );

	sprcfile << "Total de vertices:" << vertexSet_.size()<<"\n"; 
	sprcfile << "Indice Type"<<"\n";
	for(unsigned int i=0;i < vertexSet_.size(); ++i)
	{
		TeCoord2D collin = raster->coord2Index(vertexSet_[i].location());

		sprcfile << i <<" "<< vertexSet_[i].type()<<" "
						   <<collin.x()<<"  "//<<vertexSet_[i].location().x()<<"  "
						   <<collin.y()<<"  "//<<vertexSet_[i].location().y()<<"  "
						   <<vertexSet_[i].value()<<"   "
						   <<vertexSet_[i].oneEdge()<<"\n";
	}

    sprcfile << "Lista de Edges"<<"\n";
	sprcfile << "Total de Edges:" << edgeSet_.size()<<"\n"; 
	sprcfile << "Indice  FROM  TO  LEFT  RIGHT"<<"\n";
	for(unsigned int i=0;i < edgeSet_.size(); ++i)
	{
		sprcfile << i <<"  "<< edgeSet_[i].from()<<"  "<<edgeSet_[i].to()<<"  "<<edgeSet_[i].left()<<"  "<<edgeSet_[i].right()<<"\n";
	}

	sprcfile << "Lista de Triangulos"<<"\n";
	sprcfile << "Total de Triangulos:"<<triangleSet_.size()<<"\n";
	sprcfile << "Indice  Edge0  Edge1  Edge2"<<"\n";
	for (unsigned int i =0; i<triangleSet_.size(); ++i)
	{
		long e0, e1, e2;
		triangleSet_[i].edges(e0,e1,e2);
		sprcfile << i <<"  "<< e0<<"  "<< e1<<"  "<< e2<<"\n";
	}

	TeTin::TeTinTriangleIterator ti;
	for (ti = this->triangleBegin(); ti != this->triangleEnd(); ++ti)
	{
		long from, to;
		long edge0, edge1, edge2;
		long t1, t2;
		(*ti).edges(edge0, edge1, edge2);
		from = this->edge(edge0).from();
		to = this->edge(edge0).to();
		t1 = edgeSet_[edge0].left();
		t2 = edgeSet_[edge0].right();
		
		sprcfile << "Edge0: " << edge0 << "\n";
		sprcfile << "Left triangle: " << t1 << "\n";
		sprcfile << "Right triangle: " << t2 << "\n";
		sprcfile << "Vertex from: " << from << "\n";
		sprcfile << "Vertex to: " << to << "\n";
		sprcfile << "---------------------------" << "\n";
		sprcfile << "Edge1: " << edge1 << "\n";
		from = this->edge(edge1).from();
		to = this->edge(edge1).to();
		t1 = edgeSet_[edge1].left();
		t2 = edgeSet_[edge1].right();
		sprcfile << "Left triangle: " << t1 << "\n";
		sprcfile << "Right triangle: " << t2 << "\n";
		sprcfile << "Vertex from: " << from << "\n";
		sprcfile << "Vertex to: " << to << "\n";
		sprcfile << "---------------------------" << "\n";
		sprcfile << "Edge2: " << edge2 << "\n";
		from = this->edge(edge2).from();
		to = this->edge(edge2).to();
		t1 = edgeSet_[edge2].left();
		t2 = edgeSet_[edge2].right();
		sprcfile << "Left triangle: " << t1 << "\n";
		sprcfile << "Right triangle: " << t2 << "\n";
		sprcfile << "Vertex from: " << from << "\n";
		sprcfile << "Vertex to: " << to << "\n";
		sprcfile << "---------------------------" << "\n";
	}

	sprcfile.close ();
}

void
TeTin::areaTriangle (TeTinTriangle& t, float &area) //by Eduilson
{
	long v0, v1, v2;
	this->triangleVertices(t, v0, v1, v2);
	TeTinVertex vertex0, vertex1, vertex2;
	vertex0 = this->vertex(v0);
	vertex1 = this->vertex(v1);
	vertex2 = this->vertex(v2);
	//area = TriangleArea2D((float)vertex0.location().x(), (float)vertex0.location().y(),
	//	                  (float)vertex1.location().x(), (float)vertex1.location().y(),
	//					  (float)vertex2.location().x(), (float)vertex2.location().y());
}

bool 
TeTin::normalTriangle(TeTinTriangle& t, std::vector<double> &nvector) //by Eduilson (adaptado do SPRING)
{
	long v0, v1, v2;
	this->triangleVertices(t, v0, v1, v2);
	TeTinVertex vertex0, vertex1, vertex2;
	vertex0 = this->vertex(v0);
	vertex1 = this->vertex(v1);
	vertex2 = this->vertex(v2);

	nvector.clear();
	nvector.push_back(0);
	nvector.push_back(0);
	nvector.push_back(0);

	//	Define normal vector (uvx,uvy,uvz)
	double ux = vertex1.location().x() - vertex0.location().x(); 
	double vx = vertex2.location().x() - vertex0.location().x(); 
	double uy = vertex1.location().y() - vertex0.location().y(); 
	double vy = vertex2.location().y() - vertex0.location().y(); 
	double uz = vertex1.value() - vertex0.value();
	double vz = vertex2.value() - vertex0.value();

	if ( ( ux == 0 ) && ( vx == 0 ) )
	{
		nvector[0] = 1.;
		nvector[1] = 0.;
		nvector[2] = 0.;
		return true;
	}
	if ( ( uy == 0 ) && ( vy == 0 ) )
	{
		nvector[0] = 0.;
		nvector[1] = 1.;
		nvector[2] = 0.;
		return true;
	}
	if ( ( uz == 0 ) && ( vz == 0 ) )
	{
		nvector[0] = 0.;
		nvector[1] = 0.;
		nvector[2] = 1.;
		return true;
	}

	nvector[2] = (ux * vy) - (vx * uy);
	if (nvector[2] < 0.)
	{
		//Make sure that normal vector is always positive
		nvector[2] = -nvector[2];
		nvector[0] = (vy * uz) - (uy * vz);
		nvector[1] = (ux * vz) - (vx * uz);
	}
	else
	{
		nvector[0] = (uy * vz) - (vy * uz);
		nvector[1] = (vx * uz) - (ux * vz);
	}
	return true;
}

bool 
TeTin::normalTriangle(std::vector<TeTinVertex> vert, double* nvector) //by Eduilson (adaptado do SPRING)
{
	//nvector.clear();
	//nvector.push_back(0);
	//nvector.push_back(0);
	//nvector.push_back(0);
	nvector[0] = 0;
	nvector[1] = 0;

	//Define normal vector (uvx,uvy,uvz)
	double ux = vert[1].location().x() - vert[0].location().x(); 
	double vx = vert[2].location().x() - vert[0].location().x(); 
	double uy = vert[1].location().y() - vert[0].location().y(); 
	double vy = vert[2].location().y() - vert[0].location().y(); 
	double uz = vert[1].value() - vert[0].value();
	double vz = vert[2].value() - vert[0].value();

	if ( ( ux == 0 ) && ( vx == 0 ) )
	{
		nvector[0] = 1.;
		nvector[1] = 0.;
		nvector[2] = 0.;
		return true;
	}

	if ( ( uy == 0 ) && ( vy == 0 ) )
	{
		nvector[0] = 0.;
		nvector[1] = 1.;
		nvector[2] = 0.;
		return true;
	}

	if ( ( uz == 0 ) && ( vz == 0 ) )
	{
		nvector[0] = 0.;
		nvector[1] = 0.;
		nvector[2] = 1.;
		return true;
	}

	//nvector[2] = (ux * vy) - (vx * uy);
	nvector[2] = ux * vy - vx * uy;

	if (nvector[2] < 0.)
	{
		//Make sure that normal vector is always positive
		nvector[2] = -nvector[2];
		//nvector[0] = (vy * uz) - (uy * vz);
		//nvector[1] = (ux * vz) - (vx * uz);
		nvector[0] = vy * uz - uy * vz;
		nvector[1] = ux * vz - vx * uz;
	}
	else
	{
		//nvector[0] = (uy * vz) - (vy * uz);
		//nvector[1] = (vx * uz) - (ux * vz);
		nvector[0] = uy * vz - vy * uz;
		nvector[1] = vx * uz - ux * vz;
	}

	return true;
}

void
TeTin::normalizeVector(std::vector<double>& nvector)
{
	double vectorSize = sqrt ( (nvector[0]*nvector[0]) + (nvector[1]*nvector[1]) + (nvector[2]*nvector[2]) );

	if ( vectorSize != 0. )
	{
		nvector[0] = nvector[0] / vectorSize;
		nvector[1] = nvector[1] / vectorSize;
		nvector[2] = nvector[2] / vectorSize;
	}
}

void
TeTin::normalizeVector(double* nvector)
{
	double vectorSize = sqrt ( (nvector[0]*nvector[0]) + (nvector[1]*nvector[1]) + (nvector[2]*nvector[2]) );

	if ( vectorSize != 0. )
	{
		nvector[0] = nvector[0] / vectorSize;
		nvector[1] = nvector[1] / vectorSize;
		nvector[2] = nvector[2] / vectorSize;
	}
}

void
TeTin::slopeTriangle(TeTinTriangle& t,double &slope, char slopetype)
{
	slope = triangleGradient(t,'s',slopetype);
}

void
TeTin::aspectTriangle(TeTinTriangle& t, double &aspect)
{
	aspect = triangleGradient(t,'a',' ');
}


double
TeTin::triangleGradient(TeTinTriangle& t, char gradtype, char slopetype)
{
	long v0, v1, v2;
	this->triangleVertices(t, v0, v1, v2);
	TeTinVertex vertex0, vertex1, vertex2;
	vertex0 = this->vertex(v0);
	vertex1 = this->vertex(v1);
	vertex2 = this->vertex(v2);
	std::vector<double> nvector;
	double	moduv, decliv,
		pi180 = 180./3.1415927;
	double	m1, m2;
	double tol = (double).01;

	//	Special cases

	m1 = m2 = (double)TeMAXFLOAT;

	if(((double)vertex1.location().y() - (double)vertex0.location().y()) != (double)0.0)
		m1 = (vertex1.location().x() - vertex0.location().x()) / (vertex1.location().y() - vertex0.location().y());

	if(((double)vertex2.location().y() - (double)vertex0.location().y()) != (double)0.0)
		m2 = (vertex2.location().x() - vertex0.location().x()) / (vertex2.location().y() - vertex0.location().y());

	if (fabs(m1 - m2) < tol)
		//		Triangle with DUMMY Value
		return (double)TeMAXFLOAT;

	if ((vertex0.value() > TeTinBIGFLOAT) || (vertex1.value() > TeTinBIGFLOAT) || (vertex2.value() > TeTinBIGFLOAT))
		//		Triangle with DUMMY Value
		return (double)TeMAXFLOAT;

	if ((vertex0.value() == vertex1.value()) && (vertex0.value() == vertex2.value()))
	{
		if (gradtype == 's')
			//	Slope is zero
			return 0.;
		else
			//	exposition DUMMY
			return (double)TeMAXFLOAT;
	}

	this->normalTriangle(t, nvector);
	moduv = sqrt(pow(nvector[0],2.) + pow(nvector[1],2.) +
		pow(nvector[2],2.));

	if (gradtype == 's')
	{
		//Case Slope		
		if (slopetype == 'g') // degres
			decliv = pi180*acos(nvector[2]/moduv);
		else
			decliv = tan(acos(nvector[2]/moduv)) * 100.;
	}
	else
	{
		//Case Aspect		
		decliv = 90. - pi180*atan2(nvector[1],nvector[0]);
		if (decliv < 0.)
			decliv = 360. + decliv;
	}

	return decliv;
}

bool
TeTin::calculateGradient(TeRaster*& grid, char gradtype, char slopetype)
{
	this->vertexDerivatives();

	double dummy = grid->params().dummy_[0];

	double resx = grid->params().resx_;
	double resy = grid->params().resy_;

	double resx1 = grid->box().x1() + (resx / 2);
	double resy2 = grid->box().y2() - (resy / 2);

	int ncols = grid->params().ncols_;
	int nlines = grid->params().nlines_;

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Calculating slope...");
		TeProgress::instance()->setTotalSteps(nlines);
	}

	for (int line = 0; line < nlines; line++)
	{
		if (TeProgress::instance())
			TeProgress::instance()->setProgress(line);

		for (int col = 0; col < ncols; col++)
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				errorMessage_ = "Operation was cancelled.";
				return false;
			}

			TeCoord2D pt;
			pt.setXY((resx1 + ((double)col * resx)), (resy2 - ((double)line * resy)));

			TeBox boxPt(pt, pt);
			long t = this->triangleAt(boxPt);

			if (t == -1)
			{
				grid->setElement(col, line, dummy);
				continue;
			}

			TeTinTriangle triang = this->triangle(t);

			long vert[3];
			this->triangleVertices(t, vert[0], vert[1], vert[2]);

			double m1 = TeMAXFLOAT;
			double m2 = TeMAXFLOAT;

			if ((vertexSet_[vert[1]].location().y() - vertexSet_[vert[0]].location().y()) != 0.0)
				m1 = (vertexSet_[vert[1]].location().x() - vertexSet_[vert[0]].location().x()) /
					 (vertexSet_[vert[1]].location().y() - vertexSet_[vert[0]].location().y());

			if ((vertexSet_[vert[2]].location().y() - vertexSet_[vert[0]].location().y()) != 0.0)
				m2 = (vertexSet_[vert[2]].location().x() - vertexSet_[vert[0]].location().x()) /
					 (vertexSet_[vert[2]].location().y() - vertexSet_[vert[0]].location().y());

			if (fabs(m1 - m2) < 0.01)
				grid->setElement(col, line, dummy);

			double zvalue = this->triangleGradient(triang, gradtype, slopetype);

			if (zvalue > TeTinBIGFLOAT)
				grid->setElement(col, line, dummy);
			else
				grid->setElement(col, line, zvalue);
		}
	}

	TeProgress::instance()->reset();

	return true;
}

bool 
TeTin::createPointSet(TePointSet& ps, TeTable& atttable, TeLayer* layer, TeDatabase* db)
{
	//Creates an attribute table
	//Defines a list of attributes
	TeAttributeList attList;
	TeAttribute at;
	at.rep_.type_ = TeSTRING;
	at.rep_.numChar_ = 16;
	at.rep_.name_ = "object_id";
	at.rep_.isPrimaryKey_ = true;
	attList.push_back(at);

	at.rep_.type_ = TeREAL;
	at.rep_.name_ = "z";
	at.rep_.isPrimaryKey_ = false;
	at.rep_.decimals_ = 6;
	attList.push_back(at);

	atttable.setAttributeList(attList);
	atttable.setUniqueName("object_id");
	atttable.setLinkName("object_id");

	if (!layer->createAttributeTable(atttable))
		return false;

	for(unsigned int i=0;i < this->vertexSet_.size(); ++i)
	{
		if(vertexSet_[i].type() != ONBOXVERTEX)
		{
			TePoint pt(vertexSet_[i].location().x(),vertexSet_[i].location().y());
			pt.objectId(Te2String(i));
			ps.add(pt);
			TeTableRow row;
			row.push_back(Te2String(i));
			row.push_back(Te2String(vertexSet_[i].value(),6));
			atttable.add(row);
			row.clear();
		}
	}
	return true;
}

bool
TeTin::defineInterLinesColumns(TeRaster* raster, TeTinTriangle t, int& fline, int& lline, int& fcol, int& lcol)
{
	double rx1 = raster->box().x1();
	double ry2 = raster->box().y2();
	
	TeCoord2D llpt(TeCoord2D(TeMAXFLOAT, TeMAXFLOAT));
	TeCoord2D urpt(TeCoord2D(-TeMAXFLOAT, -TeMAXFLOAT));

	long v[3];
	//busca vertices do triangulo
	this->triangleVertices(t, v[0], v[1], v[2]);

	for (int i = 0; i < 3; i++)
	{
		TeTinVertex vertex = this->vertex(v[i]);

		if (vertex.type() == ONBOXVERTEX)
			return false;

		if (vertex.location() < llpt)
			llpt = vertex.location();
		if (urpt < vertex.location())
			urpt = vertex.location();
	}

	llpt.x(llpt.x() - (raster->params().resx_ / 1000));
	urpt.x(urpt.x() + (raster->params().resx_ / 1000));
	llpt.y(llpt.y() - (raster->params().resy_ / 1000));
	urpt.y(urpt.y() + (raster->params().resy_ / 1000));

	//Calculate lines and columns intercepted
	fcol = (int)((llpt.x() - rx1) / raster->params().resx_) - 1;
	lcol = (int)((urpt.x() - rx1) / raster->params().resx_) + 1;
	fline = (int)((ry2 - urpt.y()) / raster->params().resy_) - 1;
	lline = (int)((ry2 - llpt.y()) / raster->params().resy_) + 1;

	if ((raster->params().ncols_ <= fcol) || (lcol < 0) ||
		(raster->params().nlines_ <= fline) || (lline < 0))
		return false;

	return true;
}

void
TeTin::linearInterpolation(TeRaster* raster, int band, TeTinTriangle t, int fline, int lline,int fcol, int lcol)
{
	long v0,v1,v2;
	
	//busca vertices do triangulo
	this->triangleVertices(t,v0,v1,v2);
	std::list<long> vertices;

	TeTinVertex vertex0 = this->vertex(v0);
	TeTinVertex vertex1 = this->vertex(v1);
	TeTinVertex vertex2 = this->vertex(v2);

	double resx = raster->params().resx_;
	double resy = raster->params().resy_;

	double rx1 = raster->box().x1();
	double ry2 = raster->box().y2();

	double x1_x0 = vertex1.location().x() - vertex0.location().x();
	double x2_x0 = vertex2.location().x() - vertex0.location().x();
	double y1_y0 = vertex1.location().y() - vertex0.location().y();
	double y2_y0 = vertex2.location().y() - vertex0.location().y();
	double z1_z0 = vertex1.value() - vertex0.value();
	double z2_z0 = vertex2.value() - vertex0.value();

	for (int l = fline; l <= lline; l++)
	{
		for (int c = fcol; c <= lcol; c++)
		{
			TeCoord2D pt(TeCoord2D(rx1 + (c * resx), ry2 - (l * resy)));

			if (!this->triangleContainsPoint(t, pt))
				continue;

			double detx = ((y1_y0 * z2_z0) - (y2_y0 * z1_z0)) * (pt.x() - vertex0.location().x());
			double dety = ((z1_z0 * x2_x0) - (z2_z0 * x1_x0)) * (pt.y() - vertex0.location().y());
			double detz = (x1_x0 * y2_y0) - (x2_x0 * y1_y0);

			double z = ((detx + dety - (detz * vertex0.value())) / (- detz));

			raster->setElement(c, l, z, band);
		}
	}
}

bool
TeTin::linearInterpolation(TeRaster*& grid)
{
	double dummy = grid->params().dummy_[0];

	double resx = grid->params().resx_;
	double resy = grid->params().resy_;

	double resx1 = grid->box().x1() + (resx / 2);
	double resy2 = grid->box().y2() - (resy / 2);

	int ncols = grid->params().ncols_;
	int nlines = grid->params().nlines_;

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Creating MNT...");
		TeProgress::instance()->setTotalSteps(nlines);
	}

	for (int line = 0; line < nlines; line++)
	{
		if (TeProgress::instance())
			TeProgress::instance()->setProgress(line);

		for (int col = 0; col < ncols; col++)
		{
			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				errorMessage_ = "Regular grid creation was cancelled.";
				return false;
			}

			TeCoord2D pt;
			pt.setXY((resx1 + ((double)col * resx)), (resy2 - ((double)line * resy)));

			TeBox boxPt(pt, pt);
			long t = this->triangleAt(boxPt);

			if (t == -1)
			{
				grid->setElement(col, line, dummy);
				continue;
			}

			long vert[3];
			this->triangleVertices(t, vert[0], vert[1], vert[2]);

			TeTinVertex vert0 = this->vertex(vert[0]);
			TeTinVertex vert1 = this->vertex(vert[1]);
			TeTinVertex vert2 = this->vertex(vert[2]);

			if ((vert0.value() > TeTinBIGFLOAT) || (vert1.value() > TeTinBIGFLOAT) || (vert2.value() > TeTinBIGFLOAT))
			{
				grid->setElement(col, line, dummy);
				continue;
			}

			if ((vert0.value() == vert1.value()) && (vert2.value() == vert0.value()))
			{
				grid->setElement(col, line, vert0.value());
				continue;
			}

			double x1_x0 = vert1.location().x() - vert0.location().x();
			double x2_x0 = vert2.location().x() - vert0.location().x();
			double y1_y0 = vert1.location().y() - vert0.location().y();
			double y2_y0 = vert2.location().y() - vert0.location().y();
			double z1_z0 = vert1.value() - vert0.value();
			double z2_z0 = vert2.value() - vert0.value();

			double detx = ((y1_y0 * z2_z0) - (y2_y0 * z1_z0)) * (pt.x() - vert0.location().x());
			double dety = ((z1_z0 * x2_x0) - (z2_z0 * x1_x0)) * (pt.y() - vert0.location().y());
			double detz = (x1_x0 * y2_y0) - (x2_x0 * y1_y0);

			double zvalue = (detx + dety - (detz * vert0.value())) / (-detz);

			if (zvalue > TeTinBIGFLOAT)
				grid->setElement(col, line, dummy);
			else
				grid->setElement(col, line, zvalue);
		}
	}

	if (TeProgress::instance())
		TeProgress::instance()->reset();

	return true;
}

bool
TeTin::interpolationPoint(TeCoord2D &pt, double &z)
{
	long t,v0,v1,v2;
	//busca triangulo onde incide o ponto
	//t = this->triangleAt(pt);
	t = this->triangleAt(TeBox(pt, pt));
	if (t == -1)
	{
		z=-1;
		return false;
	}
	
	//busca vertices do triangulo
	this->triangleVertices(t,v0,v1,v2);
	std::list<long> vertices;
	
	//Preenche a lista com todos os vertices vizinhos ao ponto
	this->vertexOppVertices(v0,vertices);
	this->vertexOppVertices(v1,vertices);
	this->vertexOppVertices(v2,vertices);

	//Elimina vertices iguais
	vertices.sort();
	vertices.unique();

	double sum1=0, sum2=0;
	std::list<long>::iterator it_vert;
	for(it_vert = vertices.begin();it_vert!=vertices.end();it_vert++)
	{
		TeTinVertex &v = this->vertex(*it_vert);
		if(v.type() != ONBOXVERTEX ) //eliminar os vertices ONBOXVERTEX
		{
			TeCoord2D pt2(v.location().x(),v.location().y());
			double dist = TeDistance(pt,pt2);
			sum1 = sum1 + (v.value()/dist);
			sum2 = sum2 + (1/dist);
		}
	}

	z = sum1/sum2;
	return true;
}

bool 
TeTin::slopeaspectPoint(TeCoord2D &pt,double &s,double &a)
{
	//busca triangulo onde incide o ponto
	//long t = this->triangleAt(pt);
	long t = this->triangleAt(pt);
	if (t == -1)
		return false;
	this->slopeTriangle(triangle(t),s,'g');
	this->aspectTriangle(triangle(t),a);
	return true;
}

bool
TeTin::vertexOncontour(long &v)
{
	std::list<long> edges;
	std::list<long>::iterator ie;
	this->vertexEdges(v,edges);
	for(ie = edges.begin(); ie != edges.end(); ie++)
	{
		if( (edge(*ie).left() == -1) || (edge(*ie).right() == -1))
			return true;
	}
	return false;
}

bool
TeTin::createSampleSet(TeSampleSet& sample)
{
	for(unsigned int i=0;i < vertexSet_.size(); ++i)
		if(vertexSet_[i].type() != ONBOXVERTEX) 
			sample.add ( TeSample(TeCoord2D(vertexSet_[i].location().x(), vertexSet_[i].location().y()), vertexSet_[i].value()) );

	return true;
}

void
TeTin::setTolerances(double minedgesize, double isoTol, double ptsdist, double breaklineTol)
{
	minedgesize_ = minedgesize;
	isoTol_ = isoTol;
	ptsdist_ = ptsdist;
	breaklineTol_ = breaklineTol;
}

bool
TeTin::insertBreaklines(TeLineSet& breaklineSet, double scale)
{
	vsderivSet_.clear();

	this->copyBreaklines(breaklineSet);

	if (!this->insertBreaklinesPoints(breaklineSet, scale))
		return false;

	vsderivSet_.clear();

	return true;
}

void
TeTin::copyBreaklines(TeLineSet& breaklines)
{
	TeBox tinbox = this->box();

	TeLineSet lsetaux;

	TeLineSet::iterator itLines = breaklines.begin();

	while (itLines != breaklines.end())
	{
		if (itLines->size() < 2)
			continue;

		TeLine2D laux;
		int lsamples = 0;

		TeLine2D::iterator it = itLines->begin();

		while (it != itLines->end())
		{
			//Checks if line's point is inside of triangulation box
			if (TeWithin(*it, tinbox))
			{
				laux.add(*it);
				lsamples++;
			}
			else
			{
				if (lsamples > 1)
				{
					TeLineSimplify(laux, breaklineTol_, 0);// ptsdist_);
					lsetaux.add(laux);
				}

				lsamples = 0;
			}

			it++;
		}

		if (lsamples > 1)
		{
			TeLineSimplify(laux, breaklineTol_, 0);
			lsetaux.add(laux);
		}

		lsamples = 0;
		itLines++;
	}

	breaklines.clear();
	breaklines = lsetaux;
}

bool
TeTin::insertBreaklinesPoints(TeLineSet breaklines, double scale)
{
	float tol = scale;
	TeCoord2D pt1, ptf, ptn;

	if (vsderivSet_.size() == 0)
	{
		//Calculate derivatives
		this->vertexDerivatives();
	}

	TeTinVertexSet pts;
	TeTinVertex vert = TeTinVertex();

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Inserting breaklines points...");
		TeProgress::instance()->setTotalSteps(breaklines.size());
	}

	int l = 0;

	TeLineSet::iterator itLines = breaklines.begin();

	//To all breaklines
	while (itLines != breaklines.end())
	{
		if (TeProgress::instance())
			TeProgress::instance()->setProgress(l);

		if (itLines->size() < 2)
		{
			l++;
			itLines++;
			continue;
		}

		TeLine2D::iterator itPts = itLines->begin();
		pt1 = *itPts;
		ptf.setXY(pt1.x(), pt1.y());

		short flag = 0;

		itPts++;

		while (itPts != itLines->end())
		{
			pt1.setXY(itPts->x(), itPts->y());
			ptn.setXY(pt1.x(), pt1.y());

			//if ((pts.size() > 0) && (flag == 1))
			//{
			//	//Remove to avoid duplication
			//	vert = pts.last();
			//	pts.erase(vert);

			//	vert = TeTinVertex();
			//}

			flag = 1;

			//Fill list of points intersecting triangle edges
			if (!this->findInterPoints(ptf, ptn, pts))
			{
				itPts++;
				continue;
			}

			ptf.setXY(ptn.x(), ptn.y());

			itPts++;
		}

		//Mark last point of a line
		//vert = TeTinVertex();
		//vert.location().setXY(TeMAXFLOAT, TeMAXFLOAT);
		//vert.value(TeMAXFLOAT);
		//pts.add(vert);

		if (TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			errorMessage_ = "Operation was cancelled.";
			return false;
		}

		itLines++;
		l++;
	}

	if (TeProgress::instance())
		TeProgress::instance()->reset();

	//Filter of quota values
	this->point3dListFilter(pts, tol);

	fbvertex_ = vertexSet_.size() - 1;
	short frsflag = 1;

	//verttype_ = ONBREAKLINEVERTEX;

	TeTinVertexSet::iterator itVert = pts.begin();

	while (itVert != pts.end())
	{
		if (itVert->location().x() >= TeMAXFLOAT)
		{
			//If last point of a line, change type of previous vertex
			frsflag = 1;
			int nidaux = vertexSet_.size() - 2;

			if (vertexSet_[nidaux].type() == BREAKLINEFIRSTVERTEX)
				vertexSet_[nidaux].setType(NORMALVERTEX);
			else if (vertexSet_[nidaux].type() == ONBREAKLINEVERTEX)
				vertexSet_[nidaux].setType(BREAKLINELASTVERTEX);

			itVert++;
			continue;
		}

		TeCoord2D pt1 = itVert->location();
		double zvalue = itVert->value();

		vert.location().setXY(pt1.x(), pt1.y());
		vert.value(zvalue);

		if (frsflag)
		{
			//If first point of a line, set vertex type
			frsflag = 0;
			vert.setType(BREAKLINEFIRSTVERTEX);
		}
		else
			vert.setType(ONBREAKLINEVERTEX);

		verttype_ = vert.type();

		this->insertPoint(vert.location().x(), vert.location().y(), vert.value());

		itVert++;
	}

	//Check triangulation topology
	this->checkTopology();

	//Order breaklines
	this->orderEdges();

	//Recreate Delaunay
	for (long t = 1; t < triangleSet_.size() - 1; t++)
	{
		long contr = 0;
		if (!this->generateDelaunay(t, t, contr))
			return false;
	}

	if (vsderivSet_.size() > 0)
		vsderivSet_.clear();

	return true;
}

void
TeTin::vertexTriangles(long v, std::list<long>& rtri, std::list<long>& ltri)
{
	//Find one line that contains vertex
	long fedge = this->oneEdgeWithVertex(v);

	long e = fedge;

	long ltriang = edgeSet_[e].left();
	long rtriang = edgeSet_[e].right();

	if (rtriang == -1)
	{
		rtriang = ltriang;
		ltriang = -1;
	}

	//While right side triangle different from left side
	while (rtriang != ltriang)
	{
		if (edgeSet_[e].to() == v)
		{
			//If line points to node
			//insert right side triangle in right triangle list
			long t = edgeSet_[e].right();
			rtri.push_back(t);
		}
		else if (edgeSet_[e].from() == v)
		{
			//If line points to opposite direction of veretx
			//insert right side triangle in left triangle list
			long t = edgeSet_[e].left();
			ltri.push_back(t);
		}

		//Find line that contains node in the right triangle
		long edge[3];
		triangleSet_[rtriang].edges(edge[0], edge[1], edge[2]);

		long k;
		for (k = 0; k < 3; k ++)
		{
			if (edge[k] == e)
				continue;

			if ((edgeSet_[edge[k]].from() == v) ||
				(edgeSet_[edge[k]].to() == v))
				break;
		}

		if (k == 3)
			return;

		//Make right triangle equal to the triangle at the other side
		e = edge[k];
		if (edgeSet_[e].right() == rtriang)
			rtriang = edgeSet_[e].left();
		else if (edgeSet_[e].left() == rtriang)
			rtriang = edgeSet_[e].right();

		if (rtriang == -1)
		{
			rtriang = ltriang;
			ltriang = -1;
			e = fedge;
		}
	}

	//Insert left side triangle in triangle list
	if (ltriang != -1)
	{
		if (edgeSet_[e].to() == v)
		{
			long t = edgeSet_[e].right();
			rtri.push_back(t);
		}

		if (edgeSet_[e].from() == v)
		{
			long t = edgeSet_[e].left();
			ltri.push_back(t);
		}
	}
}

bool
TeTin::vertexClosestPoints(long v, std::vector<long>& clstVert)
{
	clstVert.clear();

	std::vector<double> distv;

	//Find one line that contains vertex
	long e = this->oneEdgeWithVertex(v);

	if (e == -1)
		return false;

	long fedge = e;

	for (int j = 0; j < 10; j++)
	{
		distv.push_back(TeMAXFLOAT);
		clstVert.push_back(-1);
	}

	//Find right and left triangle
	long rtri = edgeSet_[e].right();
	long ltri = edgeSet_[e].left();

	if (rtri == -1)
	{
		rtri = ltri;
		ltri = -1;
	}

	while (rtri != ltri)
	{
		long edge[3];
		triangleSet_[rtri].edges(edge[0], edge[1], edge[2]);

		long i;
		for (i = 0; i < 3; i++)
		{
			if (edge[i] == e)
				continue;

			if ((edgeSet_[edge[i]].from() == v) || (edgeSet_[edge[i]].to() == v))
				break;
		}

		if (i == 3)
			return false;

		e = edge[i];

		long vert;
		if (edgeSet_[e].from() == v)
			vert = edgeSet_[e].to();
		else
			vert = edgeSet_[e].from();

		if (vertexSet_[vert].value() < TeMAXFLOAT)
		{
			double dist =  ((vertexSet_[v].location().x() - vertexSet_[vert].location().x()) *
				(vertexSet_[v].location().x() - vertexSet_[vert].location().x())) +
				((vertexSet_[v].location().y() - vertexSet_[vert].location().y()) *
				(vertexSet_[v].location().y() - vertexSet_[vert].location().y()));

			for (long j = 0; j < 10; j++)
			{
				if (dist < distv[j])
				{
					for (long k = clstVert.size() - 1; k > j; k--)
					{
						distv[k] = distv[k-1];
						clstVert[k] = clstVert[k-1];
					}

					distv[j] = dist;
					clstVert[j] = vert;

					break;
				}
			}
		}

		//Find new right triangle
		if (edgeSet_[e].right() == rtri)
			rtri = edgeSet_[e].left();
		else if (edgeSet_[e].left() == rtri)
			rtri = edgeSet_[e].right();

		if ((rtri == -1) && (ltri != -1))
		{
			rtri = ltri;
			ltri = -1;
			e = fedge;
		}
	}

	return true;
}

bool
TeTin::breakVertexClosestPoints(long v, std::vector<long>& rclstVert, std::vector<long>& lclstVert)
{
	std::vector<double> rdistv, ldistv;

	//Find one line that contains vertex
	long e = this->oneEdgeWithVertex(v);

	if (e == -1)
		return false;

	long fedge = e;

	int clnodes = 10;

	for (int i = 0; i < clnodes; i++)
	{
		rdistv.push_back(TeMAXFLOAT);
		rclstVert.push_back(-1);

		ldistv.push_back(TeMAXFLOAT);
		lclstVert.push_back(-1);
	}

	//Find right and left triangle
	long rtri = edgeSet_[e].right();
	long ltri = edgeSet_[e].left();

	if (rtri == -1)
	{
		rtri = ltri;
		ltri = -1;
	}

	while (rtri != ltri)
	{
		long edge[3];
		triangleSet_[rtri].edges(edge[0], edge[1], edge[2]);

		long j;
		for (j = 0; j < 3; j++)
		{
			//Find line that contains vertex
			if (edge[j] == e)
				continue;
			if ((edgeSet_[edge[j]].from() == v) ||
				(edgeSet_[edge[j]].to() == v))
				break;
		}

		if (j == 3)
			return false;

		e = edge[j];
		if (edgeSet_[e].from() == v)
		{
			long vert = edgeSet_[e].to();
			if ((vert < fbvertex_) || ((breakline_ == true) && (vertexSet_[vert].type() != ONBREAKLINEVERTEX)))
			{	//Node is a breakline node at right side of it
				//if ((breakline_ == true) && ((vertexSet_[vert].type() != ONBREAKLINEVERTEX) &&
				//	(vertexSet_[vert].type() != BREAKLINEFIRSTVERTEX) && (vertexSet_[vert].type() != BREAKLINELASTVERTEX)))
				if ((breakline_ == true) && (vertexSet_[vert].type() != ONBREAKLINEVERTEX))
				{
				double dist = (vertexSet_[v].location().x() - vertexSet_[vert].location().x()) *
					(vertexSet_[v].location().x() - vertexSet_[vert].location().x()) +
					(vertexSet_[v].location().y() - vertexSet_[vert].location().y()) *
					(vertexSet_[v].location().y() - vertexSet_[vert].location().y());

				for (j = 0; j < clnodes; j++)
				{
					if (dist < rdistv[j])
					{
						for (long k = clnodes - 1; k > j; k--)
						{
							rdistv[k] = rdistv[k-1];
							rclstVert[k] = rclstVert[k-1];
						}

						rdistv[j] = dist;
						rclstVert[j] = vert;

						break;
					}
				}
				}
			}
		}
		else
		{
			long vert = edgeSet_[e].from();
			if ((vert < fbvertex_) || ((breakline_ == true) && (vertexSet_[vert].type() != ONBREAKLINEVERTEX)))
			{	//Node is a breakline node at left side of it
				//if ((breakline_ == true) && ((vertexSet_[vert].type() != ONBREAKLINEVERTEX) &&
				//	(vertexSet_[vert].type() != BREAKLINEFIRSTVERTEX) && (vertexSet_[vert].type() != BREAKLINELASTVERTEX)))
				//if ((breakline_ == true) && (vertexSet_[vert].type() != ONBREAKLINEVERTEX))
				//{
				double dist = (vertexSet_[v].location().x() - vertexSet_[vert].location().x()) *
					(vertexSet_[v].location().x() - vertexSet_[vert].location().x()) +
					(vertexSet_[v].location().y() - vertexSet_[vert].location().y()) *
					(vertexSet_[v].location().y() - vertexSet_[vert].location().y());

				for (j = 0; j < clnodes; j++)
				{
					if (dist < ldistv[j])
					{
						for (long k = clnodes - 1; k > j; k--)
						{
							ldistv[k] = ldistv[k-1];
							lclstVert[k] = lclstVert[k-1];
						}

						ldistv[j] = dist;
						lclstVert[j] = vert;

						break;
					}
				}
			//}
			}
		}

		//Find new right triangle
		if (edgeSet_[e].right() == rtri)
			rtri = edgeSet_[e].left();
		else if (edgeSet_[e].left() == rtri)
			rtri = edgeSet_[e].right();

		if ((rtri == -1) && (ltri != -1))
		{
			rtri = ltri;
			ltri = -1;
			e = fedge;
		}
	}

	return true;
}

void
TeTin::vertexDerivatives()
{
	//Calculate first derivatives on triangles
	this->triangleFirstDeriv();

	//Calculate first derivatives on vertex
	this->vertexFirstDeriv();

	//Calculate second derivatives on triangles
	this->triangleSecondDeriv();

	//Calculate second derivatives on vertex
	this->vertexSecondDeriv();

	//If there are breaklines
	if ((fbvertex_ > 0) || (breakline_ == true))
	{
		//Calculate first derivatives on breaklines vertex
		this->breakVertexFirstDeriv();

		//Calculate second derivatives on triangles that touch breaklines
		this->breakTriangleSecondDeriv();

		//Calculate second derivatives on breaklines vertex
		this->breakVertexSecondDeriv();
	}
}

void
TeTin::triangleFirstDeriv()
{
	double nvector[3];

	tfderivSet_.clear();

	//Initialize first derivatives vector
	for (long i = 0; i < triangleSet_.size() + 1; i++)
	{
		TeTinVertex vert;
		vert.location().setXY(TeMAXFLOAT, 0);
		tfderivSet_.add(vert);
	}

	std::vector<TeTinVertex> pts;

	for (long i = 0; i < 3; i++)
	{
		TeTinVertex v;
		v.location().x(0);
		v.location().y(0);
		v.value(0);

		pts.push_back(v);
	}

	TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
	TeProgress::instance()->setMessage("Calculating first derivative on triangles...");
	TeProgress::instance()->setTotalSteps(triangleSet_.size() - 2);

	for (long t = 1; t < (triangleSet_.size() - 1); t++)
	{
		TeProgress::instance()->setProgress(t - 1);

		long vert[3];
		this->triangleVertices(t, vert[0], vert[1], vert[2]);

		for (long i = 0; i < 3; i++)
		{
			pts[i].location().x(vertexSet_[vert[i]].location().x());
			pts[i].location().y(vertexSet_[vert[i]].location().y());
			pts[i].value(vertexSet_[vert[i]].value());
		}

		if ((pts[0].value() >= TeMAXFLOAT) || (pts[1].value() >= TeMAXFLOAT) || (pts[2].value() >= TeMAXFLOAT))
		{//Triangle with dummy value
			tfderivSet_[t].location().y(TeMAXFLOAT);
			continue;
		}

		double m1 = TeMAXFLOAT;
		double m2 = TeMAXFLOAT;

		if ((pts[1].location().y() - pts[0].location().y()) != 0.0)
			m1 = (pts[1].location().x() - pts[0].location().x()) / (pts[1].location().y() - pts[0].location().y());

		if ((pts[2].location().y() - pts[0].location().y()) != 0.0)
			m2 = (pts[2].location().x() - pts[0].location().x()) / (pts[2].location().y() - pts[0].location().y());

		if (fabs(m1 - m2) < 0.01)
		{
			//Triangle with dummy value
			tfderivSet_[t].location().y(TeMAXFLOAT);
			continue;
		}

		if ((pts[0].value() == pts[1].value()) && (pts[0].value() == pts[2].value()))
		{
			//Triangle parallel to XY plane
			tfderivSet_[t].location().x(0);
			continue;
		}

		//Calculate vector normal to triangle
		this->normalTriangle(pts, nvector);

		tfderivSet_[t].location().x(-nvector[0] / nvector[2]);
		tfderivSet_[t].location().y(-nvector[1] / nvector[2]);

		if (TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			errorMessage_ = "Operation was cancelled.";
			return;
		}
	}

	TeProgress::instance()->reset();
}

void
TeTin::triangleSecondDeriv()
{
	double nvector[3];

	tsderivSet_.clear();

	//Initialize second derivatives vector
	for (long i = 0; i < triangleSet_.size() + 1; i++)
	{
		TeTinVertex vert;
		vert.location().setXY(TeMAXFLOAT, TeMAXFLOAT);
		vert.value(0);
		tsderivSet_.add(vert);
	}

	TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
	TeProgress::instance()->setMessage("Calculating second derivatives on triangles...");
	TeProgress::instance()->setTotalSteps(triangleSet_.size() - 2);

	for (long t = 1; t < (triangleSet_.size() - 1); t++)
	{
		TeProgress::instance()->setProgress(t - 1);

		long vert[3];
		this->triangleVertices(t, vert[0], vert[1], vert[2]);

		//	Special case
		if ((vfderivSet_[vert[0]].location().x() >= TeMAXFLOAT) ||
			(vfderivSet_[vert[1]].location().x() >= TeMAXFLOAT) ||
			(vfderivSet_[vert[2]].location().x() >= TeMAXFLOAT) ||
			(vertexSet_[vert[0]].value() >= TeMAXFLOAT) ||
			(vertexSet_[vert[1]].value() >= TeMAXFLOAT) ||
			(vertexSet_[vert[2]].value() >= TeMAXFLOAT))
		{
			//Triangle with DUMMY Value
			tsderivSet_[t].value(TeMAXFLOAT);
			continue;
		}

		double m1 = TeMAXFLOAT;
		double m2 = TeMAXFLOAT;

		if ((vfderivSet_[vert[1]].location().y() - vfderivSet_[vert[0]].location().y()) != 0.0)
			m1 = (vfderivSet_[vert[1]].location().x() - vfderivSet_[vert[0]].location().x()) / 
				 (vfderivSet_[vert[1]].location().y() - vfderivSet_[vert[0]].location().y());

		if ((vfderivSet_[vert[2]].location().y() - vfderivSet_[vert[0]].location().y()) != 0.0)
			m2 = (vfderivSet_[vert[2]].location().x() - vfderivSet_[vert[0]].location().x()) / 
				 (vfderivSet_[vert[2]].location().y() - vfderivSet_[vert[0]].location().y());

		if (fabs(m1 - m2) < 0.01)
		{
			//Triangle with dummy value
			tsderivSet_[t].value(TeMAXFLOAT);
			continue;
		}

		std::vector<TeTinVertex> pts;

		//Calculate using dx
		for (long i = 0; i < 3; i++)
		{
			TeTinVertex v;
			v.location().x(vertexSet_[vert[i]].location().x());
			v.location().y(vertexSet_[vert[i]].location().y());
			v.value(vfderivSet_[vert[i]].location().x());

			pts.push_back(v);
		}

		double dxy, dyx;
		if ((pts[0].value() == pts[1].value()) && (pts[0].value() == pts[2].value()))
		{
			tsderivSet_[t].location().x(0);
			dxy = 0.;
		}
		else
		{
			this->normalTriangle(pts, nvector);
			tsderivSet_[t].location().x(-nvector[0] / nvector[2]);
			dxy = (-nvector[1] / nvector[2]);
		}

		//Calculate using dy
		for (long i = 0; i < 3; i++)
			pts[i].value(vfderivSet_[vert[i]].location().y());

		if ((pts[0].value() == pts[1].value()) && (pts[0].value() == pts[2].value()))
		{
			tsderivSet_[t].location().y(0.);
			dyx = 0.;
		}
		else
		{
			this->normalTriangle(pts, nvector);
			tsderivSet_[t].location().y(-nvector[1] / nvector[2]);
			dyx = (-nvector[0] / nvector[2]);
		}

		tsderivSet_[t].value((dxy + dyx) / 2.);

		if (TeProgress::instance()->wasCancelled())
		{
			errorMessage_ = "Operation was cancelled.";
			TeProgress::instance()->reset();
			return;
		}
	}

	TeProgress::instance()->reset();
}

void
TeTin::vertexFirstDeriv()
{
	std::vector<TeCoord2D> derivvec;

	//Initialize first derivatives vector
	vfderivSet_.clear();
	for (long i = 0; i < vertexSet_.size() + 1; i++)
	{
		TeTinVertex vert;
		vert.location().setXY(0, 0);
		vfderivSet_.add(vert);
	}

	TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
	TeProgress::instance()->setMessage("Calculating first derivative on vertices...");
	TeProgress::instance()->setTotalSteps(vertexSet_.size() - 2);

	for (long v = 1; v < (vertexSet_.size() - 1); v++)
	{
		TeProgress::instance()->setProgress(v - 1);

		if (vertexSet_[v].value() >= TeMAXFLOAT)
			continue;

		if (vertexSet_[v].type() == DELETEDVERTEX)
			continue;

		//Look for closest points of the vertex
		std::vector<long> clstVert;
		this->vertexClosestPoints(v, clstVert);

		TeTinVertex deriv;
		this->calcVertexFirstDeriv(v, clstVert, deriv);

		vfderivSet_[v] = deriv;

		if (TeProgress::instance()->wasCancelled())
		{
			errorMessage_ = "Operation was cancelled.";
			TeProgress::instance()->reset();
			return;
		}
	}

	TeProgress::instance()->reset();
}

void
TeTin::vertexSecondDeriv()
{
	vsderivSet_.clear();

	//Initialize second derivatives vector
	for (long i = 0; i < vertexSet_.size() + 1; i++)
	{
		TeTinVertex vert;
		vert.location().setXY(0, 0);
		vert.value(0);
		vsderivSet_.add(vert);
	}

	TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
	TeProgress::instance()->setMessage("Calculating second derivative on vertices...");
	TeProgress::instance()->setTotalSteps(vertexSet_.size() - 2);

	for (long v = 1; v < (vertexSet_.size() - 1); v++)
	{
		TeProgress::instance()->setProgress(v - 1);

		if (vertexSet_[v].value() >= TeMAXFLOAT)
			continue;
		if (vertexSet_[v].type() == DELETEDVERTEX)
			continue;

		//Look for closest point of the vertex
		std::vector<long> clstVert;
		this->vertexClosestPoints(v, clstVert);

		TeTinVertex sderiv;
		this->calcVertexSecondDeriv(v, clstVert, sderiv);

		vsderivSet_[v].location().x(sderiv.location().x());
		vsderivSet_[v].location().y(sderiv.location().y());
		vsderivSet_[v].value(sderiv.value());

		if (TeProgress::instance()->wasCancelled())
		{
			errorMessage_ = "Operation was cancelled.";
			TeProgress::instance()->reset();
			return;
		}
	}

	TeProgress::instance()->reset();
}

void
TeTin::breakVertexFirstDeriv()
{
	int bvertsize = (vertexSet_.size() - 1) - fbvertex_;

	vbrfderivSet_.clear();
	vblfderivSet_.clear();

	//Initialize first derivatives vector
	for (long i = 0; i < bvertsize + 1; i++)
	{
		vbrfderivSet_.add(vfderivSet_[i + fbvertex_]);
		vblfderivSet_.add(vfderivSet_[i + fbvertex_]);
	}

	//To each break node
	//for (long v = 1; v < (bvertsize -1); v++)
	for (long v = fbvertex_ + 1; v < ((vertexSet_.size() - 1) -1); v++)
	{
		//Special cases
		if ((vertexSet_[v].value() >= TeMAXFLOAT) ||
			(vertexSet_[v].type() != ONBREAKLINEVERTEX))
			continue;

		//Look for triangles that share the vertex
		std::vector<long> rclstVert, lclstVert;
		this->breakVertexClosestPoints(v, rclstVert, lclstVert);
		
		TeTinVertex rderiv;
		this->calcVertexFirstDeriv(v, rclstVert, rderiv);
		TeTinVertex lderiv;
		this->calcVertexFirstDeriv(v, lclstVert, lderiv);

		long v1 = v + 1;
		while ((v1 < (vertexSet_.size() - 1)) && ((vertexSet_[v1].type() != ONBREAKLINEVERTEX) &&
			(vertexSet_[v1].type() != BREAKLINEFIRSTVERTEX) && (vertexSet_[v1].type() != BREAKLINELASTVERTEX)))
			v1++;

		long v2 = v - 1;
		while ((v2 > 1) && ((vertexSet_[v2].type() != ONBREAKLINEVERTEX) &&
			(vertexSet_[v2].type() != BREAKLINEFIRSTVERTEX) && (vertexSet_[v2].type() != BREAKLINELASTVERTEX)))
			v2--;

		double deltax = vertexSet_[v1].location().x() - vertexSet_[v2].location().x();
		double deltay = vertexSet_[v1].location().y() - vertexSet_[v2].location().y();
		double modxy = sqrt((deltax * deltax) + (deltay * deltay));
		double costheta = deltax / modxy;
		double sintheta = deltay / modxy;

		double dzds;
		if (vfderivSet_[v].location().x() >= TeMAXFLOAT)
			dzds = 0.;
		else
			dzds = costheta*vfderivSet_[v].location().x() -
			sintheta*vfderivSet_[v].location().y();

		double dzdt;
		if (rderiv.location().x() >= TeMAXFLOAT)
			dzdt = 0.;
		else
			dzdt = sintheta*rderiv.location().x() +
			costheta*rderiv.location().y();

		double dzdx = costheta*dzds + sintheta*dzdt;
		double dzdy = -sintheta*dzds + costheta*dzdt;

		vbrfderivSet_[v - fbvertex_].location().x(dzdx);
		vbrfderivSet_[v - fbvertex_].location().y(dzdy);

		if (lderiv.location().x() >= TeMAXFLOAT)
			dzdt = 0.;
		else
			dzdt = sintheta*lderiv.location().x() +
			costheta*lderiv.location().y();

		dzdx = costheta*dzds + sintheta*dzdt;
		dzdy = -sintheta*dzds + costheta*dzdt;
		vblfderivSet_[v - fbvertex_].location().x(dzdx);
		vblfderivSet_[v - fbvertex_].location().y(dzdy);
	}
}

void
TeTin::breakTriangleSecondDeriv()
{
	//To each break vertex except first and last
	for (long v = fbvertex_ + 1; v < (vertexSet_.size() - 1) - 1; v++)
	{
		if (vertexSet_[v].value() >= TeMAXFLOAT)
			continue;

		if (vertexSet_[v].type() != ONBREAKLINEVERTEX)
			continue;

		std::list<long> rtri, ltri;

		//Look for triangles that share the vertex
		this->vertexTriangles(v, rtri, ltri);

		//Calculate second derivative of vertex using right side triangles
		this->calcTriangleSecondDeriv(v, rtri, vbrfderivSet_);

		//Calculate second derivative of vertex using left side triangles
		this->calcTriangleSecondDeriv(v, ltri, vblfderivSet_);

		rtri.clear();
		ltri.clear();
	}
}

void
TeTin::breakVertexSecondDeriv()
{
	int bvertsize = (vertexSet_.size() - 1) - fbvertex_;

	vbrsderivSet_.clear();
	vblsderivSet_.clear();

	//Initialize second derivatives vector
	for (long i = 0; i < (bvertsize + 1); i++)
	{
		vbrsderivSet_.add(vsderivSet_[i + fbvertex_]);
		vblsderivSet_.add(vsderivSet_[i + fbvertex_]);
	}

	//To each break node
	for (long v = fbvertex_ + 1; v < (vertexSet_.size() - 1) - 1; v++)
	{
		//Special cases
		if ((vertexSet_[v].value() >= TeMAXFLOAT) ||
			(vertexSet_[v].type() != ONBREAKLINEVERTEX))
			continue;

		//Look for triangles that share the node
		std::vector<long> rclstnids, lclstnids;
		this->breakVertexClosestPoints(v, rclstnids, lclstnids);

		TeTinVertex rsderiv;
		this->calcVertexSecondDeriv(v, rclstnids, rsderiv);
		TeTinVertex lsderiv;
		this->calcVertexSecondDeriv(v, lclstnids, lsderiv);

		long v1 = v + 1;
		while ((v1 < (vertexSet_.size() - 1)) && ((vertexSet_[v1].type() != ONBREAKLINEVERTEX) &&
			(vertexSet_[v1].type() != BREAKLINEFIRSTVERTEX) && (vertexSet_[v1].type() != BREAKLINELASTVERTEX)))
			v1++;

		long v2 = v - 1;
		while ((v2 > 1) && ((vertexSet_[v2].type() != ONBREAKLINEVERTEX) &&
			(vertexSet_[v2].type() != BREAKLINEFIRSTVERTEX) && (vertexSet_[v2].type() != BREAKLINELASTVERTEX)))
			v2--;

		double deltax = vertexSet_[v1].location().x() - vertexSet_[v2].location().x();
		double deltay = vertexSet_[v1].location().y() - vertexSet_[v2].location().y();
		double modxy = sqrt(deltax*deltax + deltay*deltay);
		double costheta = deltax / modxy;
		double sintheta = deltay / modxy;
		double cos2theta = costheta * costheta;
		double sin2theta = sintheta * sintheta;
		double sincostheta = sintheta * costheta;

		double dzdss = cos2theta*vsderivSet_[v].location().x() -
			2*sincostheta*vsderivSet_[v].value() +
			sin2theta*vsderivSet_[v].location().y();

		double dzdtt = sin2theta*rsderiv.location().x() +
			2*sincostheta*rsderiv.value() +
			cos2theta*rsderiv.location().y();

		double dzdst = sincostheta*rsderiv.location().x() +
			(cos2theta-sin2theta)*rsderiv.value() -
			sincostheta*rsderiv.location().y();

		double dzdxx = cos2theta*dzdss +
			2*sincostheta*dzdst +
			sin2theta*dzdtt;

		double dzdyy = sin2theta*dzdss -
			2*sincostheta*dzdst +
			cos2theta*dzdtt;

		double dzdxy = -sincostheta*dzdss +
			(cos2theta-sin2theta)*dzdst +
			sincostheta*dzdtt;

		vbrsderivSet_[v - fbvertex_].location().x(dzdxx);
		vbrsderivSet_[v - fbvertex_].location().y(dzdyy);
		vbrsderivSet_[v - fbvertex_].value(dzdxy);

		dzdtt = sin2theta*lsderiv.location().x() +
			2*sincostheta*lsderiv.value() +
			cos2theta*lsderiv.location().y();

		dzdst = sincostheta*lsderiv.location().x() +
			(cos2theta-sin2theta)*lsderiv.value() -
			sincostheta*lsderiv.location().y();

		dzdxx = cos2theta*dzdss +
			2*sincostheta*dzdst +
			sin2theta*dzdtt;

		dzdyy = sin2theta*dzdss -
			2*sincostheta*dzdst +
			cos2theta*dzdtt;

		dzdxy = -sincostheta*dzdss +
			(cos2theta-sin2theta)*dzdst +
			sincostheta*dzdtt;

		vblsderivSet_[v - fbvertex_].location().x(dzdxx);
		vblsderivSet_[v - fbvertex_].location().y(dzdyy);
		vblsderivSet_[v - fbvertex_].value(dzdxy);
	}
}

void
TeTin::calcVertexFirstDeriv(long v, std::vector<long> clstVertex, TeTinVertex& deriv)
{
	double nvector[3];

	TeTinVertex vert;
	vert.location().setXY(0, 0);
	vert.value(0);

	std::vector<TeTinVertex> verts;

	verts.push_back(vertexSet_[v]);
	verts.push_back(vert);
	verts.push_back(vert);

	double tnx = 0;
	double tny = 0;
	double tnz = 0;

	for (long i = 0; i < clstVertex.size(); i++)
	{
		if (clstVertex[i] == -1)
			break;

		verts[1].location().setXY(vertexSet_[clstVertex[i]].location().x(), vertexSet_[clstVertex[i]].location().y());
		verts[1].value(vertexSet_[clstVertex[i]].value());

		for (long j = i + 1; j < clstVertex.size(); j++)
		{
			if (clstVertex[j] == -1)
				break;

			verts[2].location().setXY(vertexSet_[clstVertex[j]].location().x(), vertexSet_[clstVertex[j]].location().y());
			verts[2].value(vertexSet_[clstVertex[j]].value());

			double m1 = TeMAXFLOAT;
			double m2 = TeMAXFLOAT;

			if((verts[1].location().y() - verts[0].location().y()) != 0.0)
				m1 = (verts[1].location().x() - verts[0].location().x()) / (verts[1].location().y() - verts[0].location().y());

			if((verts[2].location().y() - verts[0].location().y()) != 0.0)
				m2 = (verts[2].location().x() - verts[0].location().x()) / (verts[2].location().y() - verts[0].location().y());

			if (fabs(m1 - m2) < 0.01)
				continue;

			if ((verts[0].value() >= TeMAXFLOAT) || (verts[1].value() >= TeMAXFLOAT) || (verts[2].value() >= TeMAXFLOAT))
				continue;

			if ((verts[0].value() == verts[1].value()) && (verts[0].value() == verts[2].value()))
				continue;

			this->normalTriangle(verts, nvector);

			tnx += nvector[0];
			tny += nvector[1];
			tnz += nvector[2];
		}
	}

	//Calculate node first derivatives
	if (tnz != 0.)
	{
		deriv.location().x(-tnx/tnz);
		deriv.location().y(-tny/tnz);
	}
	else
	{
		deriv.location().x(0.);
		deriv.location().y(0.);
	}
}

void
TeTin::calcVertexSecondDeriv(long v, std::vector<long> clstVertex, TeTinVertex& deriv)
{
	double nvector[3];
	std::vector<TeTinVertex> verts;

	TeTinVertex vert;
	vert.location().setXY(vertexSet_[v].location().x(), vertexSet_[v].location().y());
	vert.value(vfderivSet_[v].location().x());
	verts.push_back(vert);

	vert.location().setXY(0, 0);
	vert.value(0);
	verts.push_back(vert);
	verts.push_back(vert);

	double tnxx = 0.;
	double tnxy = 0.;
	double tnxz = 0.;
	double tnyx = 0.;
	double tnyy = 0.;
	double tnyz = 0.;

	for (long i = 0; i < clstVertex.size(); i++)
	{
		if (clstVertex[i] == -1)
			break;

		verts[1].location().setXY(vertexSet_[clstVertex[i]].location().x(), vertexSet_[clstVertex[i]].location().y());
		verts[1].value(vfderivSet_[clstVertex[i]].location().x());

		for (long j = i + 1; j < clstVertex.size(); j++)
		{
			if (clstVertex[j] == -1)
				break;

			verts[2].location().setXY(vertexSet_[clstVertex[j]].location().x(), vertexSet_[clstVertex[j]].location().y());
			verts[2].value(vfderivSet_[clstVertex[j]].location().x());

			double m1 = TeMAXFLOAT;
			double m2 = TeMAXFLOAT;

			if((verts[1].location().y() - verts[0].location().y()) != 0.0)
				m1 = (verts[1].location().x() - verts[0].location().x()) / (verts[1].location().y() - verts[0].location().y());

			if((verts[2].location().y() - verts[0].location().y()) != 0.0)
				m2 = (verts[2].location().x() - verts[0].location().x()) / (verts[2].location().y() - verts[0].location().y());

			if (fabs(m1 - m2) < 0.01)	
				continue;

			if ((verts[0].value() >= TeMAXFLOAT) ||
				(verts[1].value() >= TeMAXFLOAT) ||
				(verts[2].value() >= TeMAXFLOAT))
				continue;	//Triangle with dummy value

			if ((verts[0].value() == verts[1].value()) &&
				(verts[0].value() == verts[2].value()))
				continue;	//Triangle parallel to XY plane

			this->normalTriangle(verts, nvector);

			tnxx += nvector[0];
			tnxy += nvector[1];
			tnxz += nvector[2];
		}
	}

	verts[0].value(vfderivSet_[v].location().y());
	for (long j = 0; j < clstVertex.size(); j++)
	{
		if (clstVertex[j] == -1)
			break;

		verts[1].location().x(vertexSet_[clstVertex[j]].location().x());
		verts[1].location().y(vertexSet_[clstVertex[j]].location().y());
		verts[1].value(vfderivSet_[clstVertex[j]].location().y());

		for (long k = j+1; k < clstVertex.size(); k++)
		{
			if (clstVertex[k] == -1)
				break;

			verts[2].location().x(vertexSet_[clstVertex[k]].location().x());
			verts[2].location().y(vertexSet_[clstVertex[k]].location().y());
			verts[2].value(vfderivSet_[clstVertex[k]].location().y());

			double m1 = TeMAXFLOAT;
			double m2 = TeMAXFLOAT;

			if((verts[1].location().y() - verts[0].location().y()) != 0.0)
				m1 = (verts[1].location().x() - verts[0].location().x()) / (verts[1].location().y() - verts[0].location().y());

			if((verts[2].location().y() - verts[0].location().y()) != 0.0)
				m2 = (verts[2].location().x() - verts[0].location().x()) / (verts[2].location().y() - verts[0].location().y());

			if (fabs(m1 - m2) < 0.01)	
				continue;

			if ((verts[0].value() >= TeMAXFLOAT) ||
				(verts[1].value() >= TeMAXFLOAT) ||
				(verts[2].value() >= TeMAXFLOAT))
				continue;

			if ((verts[0].value() == verts[1].value()) &&
				(verts[0].value() == verts[2].value()))
				continue;

			this->normalTriangle(verts, nvector);
			tnyx += nvector[0];
			tnyy += nvector[1];
			tnyz += nvector[2];
		}
	}

	//Calculate vertex second derivatives
	if (tnxz != 0.)
	{
		deriv.location().x(-tnxx/tnxz);
		tnxy = -tnxy / tnxz;
	}
	else
		tnxy = 0.;

	if (tnyz != 0.)
	{
		deriv.location().y(-tnyy/tnyz);
		tnyx = -tnyx/tnyz;
	}
	else
		tnyx = 0.;

	deriv.value((tnxy + tnyx) / 2.);
}

void
TeTin::calcTriangleSecondDeriv(long v, std::list<long> triangles, TeTinVertexSet& fderiv)
{
	double nvector[3];

	std::list<long>::iterator tri = triangles.begin();

	while (tri != triangles.end())
	{
		long t = *tri;

		if (t == -1)
		{
			tri++;
			continue;
		}

		long vert[3];
		this->triangleVertices(t, vert[0], vert[1], vert[2]);

		if ((vertexSet_[vert[0]].value() >= TeMAXFLOAT) ||
			(vertexSet_[vert[1]].value() >= TeMAXFLOAT) ||
			(vertexSet_[vert[2]].value() >= TeMAXFLOAT))
		{
			//Triangle with DUMMY Value
			tsderivSet_[t].location().x(TeMAXFLOAT);
			tsderivSet_[t].location().y(TeMAXFLOAT);
			tsderivSet_[t].value(TeMAXFLOAT);

			tri++;
			continue;
		}

		double m1 = TeMAXFLOAT;
		double m2 = TeMAXFLOAT;

		if((vertexSet_[vert[1]].location().y() - vertexSet_[vert[0]].location().y())!= 0.0)
		{
			m1 = (vertexSet_[vert[1]].location().x() - vertexSet_[vert[0]].location().x()) / 
				 (vertexSet_[vert[1]].location().y() - vertexSet_[vert[0]].location().y());
		}

		if((vertexSet_[vert[2]].location().y() - vertexSet_[vert[0]].location().y()) != 0.0)
		{
			m2 = (vertexSet_[vert[2]].location().x() - vertexSet_[vert[0]].location().x()) / 
				 (vertexSet_[vert[2]].location().y() - vertexSet_[vert[0]].location().y());
		}

		if (fabs(m1 - m2) < 0.01)
		{
			//Triangle with DUMMY Value
			tsderivSet_[t].location().x(TeMAXFLOAT);
			tsderivSet_[t].location().y(TeMAXFLOAT);
			tsderivSet_[t].value(TeMAXFLOAT);

			tri++;
			continue;
		}

		std::vector<TeTinVertex> pts;

		//Calculate using dx
		for (int j = 0; j < 3; j++)
		{
			TeTinVertex v;
			v.location().x(vertexSet_[vert[j]].location().x());
			v.location().y(vertexSet_[vert[j]].location().y());

			pts.push_back(v);

			//If breakline node
			//if ((vertexSet_[vert[j]].type() == ONBREAKLINEVERTEX) ||
			//	(vertexSet_[vert[j]].type() == BREAKLINEFIRSTVERTEX) ||
			//	(vertexSet_[vert[j]].type() == BREAKLINELASTVERTEX))
			if (vertexSet_[vert[j]].type() == ONBREAKLINEVERTEX)
			{
				pts[j].value(fderiv[vert[j] - fbvertex_].location().x());
			}
			else
				pts[j].value(vfderivSet_[vert[j]].location().x());
		}

		double dxy, dyx;

		if ((pts[0].value() == pts[1].value()) && (pts[0].value() == pts[2].value()))
		{
			tsderivSet_[t].location().x(0.);
			dxy = 0.;
		}
		else
		{
			this->normalTriangle(pts, nvector);

			tsderivSet_[t].location().x(-nvector[0] / nvector[2]);
			dxy = (-nvector[1] / nvector[2]);
		}

		//Calculate using dy
		for (int j = 0; j < 3; j++)
		{
			//if (vertexSet_[vert[j]].type() > 2  && vertexSet_[vert[j]].type() < 6)
			//	//				If breakline node		
			//	pts[j].value(fderiv[vert[j] - fbvertex_].location().y());
			//else
			//	pts[j].value(vfderivSet_[vert[j]].location().y());

			//If breakline node
			//if ((vertexSet_[vert[j]].type() == ONBREAKLINEVERTEX) ||
			//	(vertexSet_[vert[j]].type() == BREAKLINEFIRSTVERTEX) ||
			//	(vertexSet_[vert[j]].type() == BREAKLINELASTVERTEX))
			if (vertexSet_[vert[j]].type() == ONBREAKLINEVERTEX)
			{
				pts[j].value(fderiv[vert[j] - fbvertex_].location().y());
			}
			else
				pts[j].value(vfderivSet_[vert[j]].location().y());
		}

		if ((pts[0].value() == pts[1].value()) && (pts[0].value() == pts[2].value()))
		{
			tsderivSet_[t].location().y(0.);
			dyx = 0.;
		}
		else
		{
			this->normalTriangle(pts, nvector);

			tsderivSet_[t].location().y(-nvector[1] / nvector[2]);
			dyx = (-nvector[0] / nvector[2]);
		}
		
		tsderivSet_[t].value((double)((dxy + dyx)/2.));

		tri++;
	}
}

bool
TeTin::findInterPoints(TeCoord2D& pf, TeCoord2D& pn, TeTinVertexSet& pts)
{
	TeTinVertex p3t1, p3t2;
	TeCoord2D pt, minpt;
	long ftri, ltri, nids[3], lids[3],
		ledge, ntri;
	int i, j, k, auxj, minj;
	double tol = minedgesize_;
	double dist, mindist;

//	Search triangle containing first point
	//ftri = this->triangleAt(pf);
	ftri = this->triangleAt(TeBox(pf, pf));

	if (ftri == -1)
		return false;

//	Search triangle containing last point
	//ltri = this->triangleAt(pn);
	ltri = this->triangleAt(TeBox(pn, pn));
	if (ltri == -1)
		return false;

	if (ftri == ltri)
	{
		if (ftri == -1)
			return false;

//		If first triangle equal to last
		triangleSet_[ftri].edges(lids[0], lids[1], lids[2]);
		p3t1 = TeTinVertex();
		p3t2 = TeTinVertex();
		p3t1.location().setXY(pf.x(), pf.y());
		p3t2.location().setXY(pn.x(), pn.y());
		for (i = 0; i < 3; i++)
			if ((vertexSet_[edgeSet_[lids[i]].from()].value() >= TeMAXFLOAT) ||
				(vertexSet_[edgeSet_[lids[i]].to()].value() >= TeMAXFLOAT))
			{
				p3t1.value(TeMAXFLOAT);
				p3t2.value(TeMAXFLOAT);
				break;
			}

//		Calculate zvalue of first and last points
		if (i == 3)
		{
			for (k = 0; k < 3; k++)
				if (this->onIsolineSegment(lids[k], p3t1))
//				If on isoline segment
					break;

			for (k = 0; k < 3; k++)
				if (this->onIsolineSegment(lids[k], p3t2))
//				If on isoline segment
					break;

//			Calculate zvalue of first and last points using quintic surface
			//if ((p3t1.geomId() == 0) && (p3t2.geomId() == 0))		
				this->calcZvalueAkima(ftri, p3t1, p3t2);
			//else if (p3t1.geomId() == 0)
			//	this->calcZvalueAkima(ftri, p3t1, p3t1);
			//else if (p3t2.geomId() == 0)
			//	this->calcZvalueAkima(ftri, p3t2, p3t2);
		}

//		Insert points in list
		pts.add(p3t1);
		pts.add(p3t2);

		return true;
	}

//	Search intersecting edge and point between first triangle
//	 edges and the segment from first to last point.
	triangleSet_[ftri].edges(lids[0], lids[1], lids[2]);
	this->triangle3Neighbors(ftri, nids[0], nids[1], nids[2]);
	pt.x(TeMAXFLOAT);
	auxj = -1;
	for (j = 0; j < 3; j++)
	{
		TeCoord2D pt1 = vertexSet_[edgeSet_[lids[j]].from()].location();
		TeCoord2D pt2 = vertexSet_[edgeSet_[lids[j]].to()].location();
		if (TeSegmentsIntersectPoint(pf, pn, pt1, pt2, pt))
		{
			if (pt == pt2 || pt == pt1)
			{
				auxj = j;
				minpt = pt;
			}
			else
				break;
		}
	}
	if ((j == 3) && (auxj != -1))
	{
		j = auxj;
		pt = minpt;
	}
	if (j == 3)
	{
		minj = -1;
		mindist = TeMAXFLOAT;
		for (j = 0; j < 3; j++)
		{
			TeCoord2D pt1 = vertexSet_[edgeSet_[lids[j]].from()].location();
			TeCoord2D pt2 = vertexSet_[edgeSet_[lids[j]].to()].location();
			TeCoord2D pti;
			dist = TePerpendicularDistance(pt1, pt2, pf, pti);
			if (dist < tol)
			{
				if (dist < mindist)
				{
					minj = j;
					mindist = dist;
					minpt.x(pf.x());
					minpt.y(pf.y());
				}
				if (this->onSameSide(pf, pn, pt1, pt2) != 1)
				{
					pt.x(pf.x());
					pt.y(pf.y());
					break;
				}
			}
			dist = TePerpendicularDistance(pt1, pt2, pn, pti);
			if (dist < tol)
			{
				if (dist < mindist)
				{
					minj = j;
					mindist = dist;
					minpt.x(pn.x());
					minpt.y(pn.y());
				}
				if (this->onSameSide(pf, pn, pt1, pt2) != 1)
				{
					pt.x(pn.x());
					pt.y(pn.y());
					break;
				}
			}
			dist = TePerpendicularDistance(pf, pn, pt1, pti);
			if (dist < tol)
			{
				if (dist < mindist)
				{
					minj = j;
					mindist = dist;
					minpt.x(pt1.x());
					minpt.y(pt1.y());
				}
				if (this->onSameSide(pf, pn, pt1, pt2) != 1)
				{
					pt.x(pt1.x());
					pt.y(pt1.y());
					break;
				}
			}
			dist = TePerpendicularDistance(pf, pn, pt2, pti);
			if (dist < tol)
			{
				if (dist < mindist)
				{
					minj = j;
					mindist = dist;
					minpt.x(pt2.x());
					minpt.y(pt2.y());
				}
				if (this->onSameSide(pf, pn, pt1, pt2) != 1)
				{
					pt.x(pt2.x());
					pt.y(pt2.y());
					break;
				}
			}
		}
		if (minj != -1)
		{
			j = minj;
			pt.x(minpt.x());
			pt.y(minpt.y());
		}
	}

	//if (j == 3)
	//	return false;

	p3t1 = TeTinVertex();
	p3t2 = TeTinVertex();

	p3t1.location().setXY(pf.x(), pf.y());
	p3t2.location().setXY(pt.x(), pt.y());
	for (i = 0; i < 3; i++)
		if ((vertexSet_[edgeSet_[lids[i]].from()].value() >= TeMAXFLOAT) ||
			(vertexSet_[edgeSet_[lids[i]].to()].value() >= TeMAXFLOAT))
		{
			p3t1.value(TeMAXFLOAT);
			p3t2.value(TeMAXFLOAT);
			break;
		}
//	Calculate zvalue of first and intersected points
	if (i == 3)
	{
		for (k = 0; k < 3; k++)
			if (this->onIsolineSegment(lids[k], p3t1))
//			If on isoline segment
				break;

		for (k = 0; k < 3; k++)
			if (this->onIsolineSegment(lids[k], p3t2))
//			If on isoline segment
				break;

//		Calculate zvalue of first and last points using quintic surface
		//if ((p3t1.geomId() == 0) && (p3t2.geomId() == 0))		
			this->calcZvalueAkima(ftri, p3t1, p3t2);
		//else if (p3t1.geomId() == 0)
		//	this->calcZvalueAkima(ftri, p3t1, p3t1);
		//else if (p3t2.geomId() == 0)
		//	this->calcZvalueAkima(ftri, p3t2, p3t2);
	}

//	Insert points in list
	pts.add(p3t1);
	pts.add(p3t2);

	k = 0;
	while (nids[j] != ltri)
	{
		ledge = lids[j];
		ntri = nids[j];

		if (ntri == -1)
			break;

		triangleSet_[ntri].edges(lids[0], lids[1], lids[2]);
		this->triangle3Neighbors(ntri, nids[0],nids[1], nids[2]);

//		Search intersecting edge and point in triangle
		pt.x(TeMAXFLOAT);
		auxj = -1;
		for (j = 0; j < 3; j++)
		{
			if (lids[j] == ledge)
				continue;

			TeCoord2D pt1 = vertexSet_[edgeSet_[lids[j]].from()].location();
			TeCoord2D pt2 = vertexSet_[edgeSet_[lids[j]].to()].location();
			if (TeSegmentsIntersectPoint(pf, pn, pt1, pt2, pt))
			{
				if (pt == pt2 || pt == pt1)
				{
					auxj = j;
					minpt = pt;
				}
				else
					break;
			}
		}
		if ((j == 3) && (auxj != -1))
		{
			j = auxj;
			pt = minpt;
		}
		if (j == 3)
		{
			minj = -1;
			mindist = TeMAXFLOAT;
			for (j = 0; j < 3; j++)
			{
				if (lids[j] == ledge)
					continue;

				TeCoord2D pt1 = vertexSet_[edgeSet_[lids[j]].from()].location();
				TeCoord2D pt2 = vertexSet_[edgeSet_[lids[j]].to()].location();
				TeCoord2D pti;
				dist = TePerpendicularDistance(pt1, pt2, pf, pti);
				if (dist < tol)
				{
					if (dist < mindist)
					{
						minj = j;
						mindist = dist;
						minpt.x(pf.x());
						minpt.y(pf.y());
					}
					if (this->onSameSide(pf, pn, pt1, pt2) != 1)
					{
						pt.x(pf.x());
						pt.y(pf.y());
						break;
					}
				}
				dist = TePerpendicularDistance(pt1, pt2, pn, pti);
				if (dist < tol)
				{
					if (dist < mindist)
					{
						minj = j;
						mindist = dist;
						minpt.x(pn.x());
						minpt.y(pn.y());
					}
					if (this->onSameSide(pf, pn, pt1, pt2) != 1)
					{
						pt.x(pn.x());
						pt.y(pn.y());
						break;
					}
				}
				dist = TePerpendicularDistance(pf, pn, pt1,pti);
				if (dist < tol)
				{
					if (dist < mindist)
					{
						minj = j;
						mindist = dist;
						minpt.x(pt1.x());
						minpt.y(pt1.y());
					}
					if (this->onSameSide(pf, pn, pt1, pt2) != 1)
					{
						pt.x(pt1.x());
						pt.y(pt1.y());
						break;
					}
				}
				dist = TePerpendicularDistance(pf, pn, pt2, pti);
				if (dist < tol)
				{
					if (dist < mindist)
					{
						minj = j;
						mindist = dist;
						minpt.x(pt2.x());
						minpt.y(pt2.y());
					}
					if (this->onSameSide(pf, pn, pt1, pt2) != 1)
					{
						pt.x(pt2.x());
						pt.y(pt2.y());
						break;
					}
				}
			}
			if (minj != -1)
			{
				j = minj;
				pt.x(minpt.x());
				pt.y(minpt.y());
			}
			if (j == 3)
			{
				return false;
			}
		}

		if (k == 0)
		{
//			Search next intersecting point
			p3t1 = TeTinVertex();

			if (this->isIsolineSegment(lids[j]))
			{
				//p3t1.geomId(1);
				p3t1.value(vertexSet_[edgeSet_[lids[j]].to()].value());
			}
			//else
			//	p3t1.geomId(0);

			p3t1.location().setXY(pt.x(), pt.y());
			k++;
		}
		else
		{
			p3t2 = TeTinVertex();

			if (this->isIsolineSegment(lids[j]))
			{
				//p3t2.geomId(1);
				p3t2.value(vertexSet_[edgeSet_[lids[j]].to()].value());
			}
			//else
				//p3t2.geomId(0);

			p3t2.location().setXY(pt.x(), pt.y());

			for (i = 0; i < 3; i++)
				if ((vertexSet_[edgeSet_[lids[i]].from()].value() >= TeMAXFLOAT) ||
					(vertexSet_[edgeSet_[lids[i]].to()].value() >= TeMAXFLOAT))
				{
					p3t1.value(TeMAXFLOAT);
					p3t2.value(TeMAXFLOAT);
					break;
				}

//			Calculate zvalue of intersected point using quintic surface
			if (i == 3)
			{
//				Calculate zvalue of first and last points using quintic surface
				//if ((p3t1.geomId() == 0) && (p3t2.geomId() == 0))		
					this->calcZvalueAkima(ntri, p3t1, p3t2);
				//else if (p3t1.geomId() == 0)
				//	this->calcZvalueAkima(ntri, p3t1, p3t1);
				//else if (p3t2.geomId() == 0)
				//	this->calcZvalueAkima(ntri, p3t2, p3t2);
			}
			pts.add(p3t1);
			pts.add(p3t2);
			k = 0;
		}
	}
	triangleSet_[ltri].edges(lids[0], lids[1], lids[2]);
	if (k == 0)
	{
		p3t1 = TeTinVertex();
		
		p3t1.location().setXY(pn.x(), pn.y());
		for (i = 0; i < 3; i++)
			if ((vertexSet_[edgeSet_[lids[i]].from()].value() >= TeMAXFLOAT) ||
				(vertexSet_[edgeSet_[lids[i]].to()].value() >= TeMAXFLOAT))
			{
				p3t1.value(TeMAXFLOAT);
				break;
			}

//		Calculate zvalue of last point using quintic surface
		if (i == 3)
		{
			for (k = 0; k < 3; k++)
				if (this->onIsolineSegment(lids[k], p3t1))
//					If on isoline segment
					break;

//			Calculate zvalue of first and last points using quintic surface
			//if (p3t1.geomId() == 0)		
				this->calcZvalueAkima(ltri, p3t1, p3t1);
		}
		pts.add(p3t1);
	}
	else
	{
		p3t2 = TeTinVertex();
		p3t2.location().setXY(pn.x(), pn.y());
		
		for (i = 0; i < 3; i++)
			if ((vertexSet_[edgeSet_[lids[i]].from()].value() >= TeMAXFLOAT) ||
				(vertexSet_[edgeSet_[lids[i]].to()].value() >= TeMAXFLOAT))
			{
				p3t1.value(TeMAXFLOAT);
				p3t2.value(TeMAXFLOAT);
				break;
			}

		if (i == 3)
		{
			for (k = 0; k < 3; k++)
				if (this->onIsolineSegment(lids[k], p3t1))
//					If on isoline segment
					break;

			for (k = 0; k < 3; k++)
				if (this->onIsolineSegment(lids[k], p3t2))
//					If on isoline segment
					break;

//			Calculate zvalue of first and last points using quintic surface
			//if (p3t2.geomId() == 0)		
				this->calcZvalueAkima(ltri, p3t1, p3t2);
		}
		pts.add(p3t1);
		pts.add(p3t2);
	}

	return true;
}

void
TeTin::calcZvalueAkima(long t, TeTinVertex& v1, TeTinVertex& v2)
{
	double coef[27];

	this->defineAkimaCoeficients(t, coef);

	//Polynomial coefficients
	double p00 = coef[0];	double p01 = coef[1];	double p02 = coef[2];	double p03 = coef[3];	double p04 = coef[4];	double p05 = coef[5];
	double p10 = coef[6];	double p11 = coef[7];	double p12 = coef[8];	double p13 = coef[9];	double p14 = coef[10];
	double p20 = coef[11];	double p21 = coef[12];	double p22 = coef[13];	double p23 = coef[14];
	double p30 = coef[15];	double p31 = coef[16];	double p32 = coef[17];
	double p40 = coef[18];	double p41 = coef[19];
	double p50 = coef[20];

	//Coefficients of conversion from XY to UV coordinates
	double ap = coef[21]; double bp = coef[22]; double cp = coef[23]; double dp = coef[24];     
	double x0 = coef[25]; double y0 = coef[26];

	//Converts point from XY to UV
	double u = (ap * (v1.location().x() - x0)) + (bp * (v1.location().y() - y0));
	double v = (cp * (v1.location().x() - x0)) + (dp * (v1.location().y() - y0));

	//Evaluates the polynomial
	double p0 = p00 + (v * (p01 + (v * (p02 + (v * (p03 + (v * (p04 + (v * p05)))))))));
	double p1 = p10 + (v * (p11 + (v * (p12 + (v * (p13 + (v * p14)))))));
	double p2 = p20 + (v * (p21 + (v * (p22 + (v * p23)))));
	double p3 = p30 + (v * (p31 + (v * p32)));
	double p4 = p40 + (v * p41);

	double z = p0 + (u * (p1 + (u * (p2 + (u * (p3 + (u * (p4 + (u * p50)))))))));
	v1.value(z);

	//Converts point from XY to UV
	u = (ap * (v2.location().x() - x0)) + (bp * (v2.location().y() - y0));
	v = (cp * (v2.location().x() - x0)) + (dp * (v2.location().y() - y0));

	//Evaluates the polynomial
	p0 = p00 + (v * (p01 + (v * (p02 + (v * (p03 + (v * (p04 + (v * p05)))))))));
	p1 = p10 + (v * (p11 + (v * (p12 + (v * (p13 + (v * p14)))))));
	p2 = p20 + (v * (p21 + (v * (p22 + (v * p23)))));
	p3 = p30 + (v * (p31 + (v * p32)));
	p4 = p40 + (v * p41);

	z = p0 + (u * (p1 + (u * (p2 + (u * (p3 + (u * (p4 + (u * p50)))))))));
	v2.value(z);
}

void
TeTin::defineAkimaCoeficients(long t, double* coef)
{
	long verts[3];
	this->triangleVertices(t, verts[0], verts[1], verts[2]);

	TeTinVertexSet vertSet;

	for (int i = 0; i < 3; i++)
	{
		TeTinVertex vert;
		vert.location().setXY(vertexSet_[verts[i]].location().x(),
							  vertexSet_[verts[i]].location().y());
		vert.value(vertexSet_[verts[i]].value());
		vertSet.add(vert);
	}

	this->defineAkimaCoeficients(t, vertSet, coef);
}

void
TeTin::defineAkimaCoeficients(long t, TeTinVertexSet vertSet, double* coef)
{
	long v;
	long vert[3];
	this->triangleVertices(t, vert[0], vert[1], vert[2]);

	//Coeficients of conversion from UV to XY coordinates
	double a = vertSet[1].location().x() - vertSet[0].location().x();
	double b = vertSet[2].location().x() - vertSet[0].location().x();
	double c = vertSet[1].location().y() - vertSet[0].location().y();
	double d = vertSet[2].location().y() - vertSet[0].location().y();

	double aa = a * a;
	double bb = b * b;
	double cc = c * c;
	double dd = d * d;

	//Coeficients of conversion from XY to UV coordinates
	double ad = a * d;
	double bc = b * c;	
	double det = ad - bc;
	double ap = d / det;
	double bp = -b / det;
	double cp = -c / det;
	double dp = a / det;

	int bside = 0;
	if (fbvertex_ > 0)
	{
		//If there are breaklines
		long e[3];
		triangleSet_[t].edges(e[0], e[1], e[2]);

		for (int i = 0; i < 3; i++)
		{
			if ((edgeSet_[e[i]].to() >= fbvertex_) && (edgeSet_[e[i]].from() >= fbvertex_))
			{
				if (edgeSet_[e[i]].right() == t)//Triangle at the right side of a breakline
					bside = 1;
				else if (edgeSet_[e[i]].left() == t)//Triangle at the left side of a breakline
					bside = 2;

				break;
			}
			if (edgeSet_[e[i]].to() >= fbvertex_)
			{
				//Triangle at the right side of a breakline
				bside = 1;
				break;
			}
			if (edgeSet_[e[i]].from() >= fbvertex_)
			{
				//Triangle at the left side of a breakline
				bside = 2;
				break;
			}
		}
	}
	else if (breakline_ == true)
	{
		//If there are breaklines
		long e[3];
		triangleSet_[t].edges(e[0], e[1], e[2]);

		for (int i = 0; i < 3; i++)
		{
			//if (((this->vertex(edgeSet_[e[i]].to()).type() == ONBREAKLINEVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].to()).type() == BREAKLINEFIRSTVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].to()).type() == BREAKLINELASTVERTEX)) &&
			//	((this->vertex(edgeSet_[e[i]].from()).type() == ONBREAKLINEVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].from()).type() == BREAKLINEFIRSTVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].from()).type() == BREAKLINELASTVERTEX)))
			if ((this->vertex(edgeSet_[e[i]].to()).type() == ONBREAKLINEVERTEX) &&
				(this->vertex(edgeSet_[e[i]].from()).type() == ONBREAKLINEVERTEX))
			{
				if (edgeSet_[e[i]].right() == t)//Triangle at the right side of a breakline
					bside = 1;
				else if (edgeSet_[e[i]].left() == t)//Triangle at the left side of a breakline
					bside = 2;

				break;
			}
			//if ((this->vertex(edgeSet_[e[i]].to()).type() == ONBREAKLINEVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].to()).type() == BREAKLINEFIRSTVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].to()).type() == BREAKLINELASTVERTEX))
			if (this->vertex(edgeSet_[e[i]].to()).type() == ONBREAKLINEVERTEX)
			{
				//Triangle at the right side of a breakline
				bside = 1;
				break;
			}
			//if ((this->vertex(edgeSet_[e[i]].from()).type() == ONBREAKLINEVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].from()).type() == BREAKLINEFIRSTVERTEX) ||
			//	(this->vertex(edgeSet_[e[i]].from()).type() == BREAKLINELASTVERTEX))
			if (this->vertex(edgeSet_[e[i]].from()).type() == ONBREAKLINEVERTEX)
			{
				//Triangle at the left side of a breakline
				bside = 2;
				break;
			}
		}
	}

	double zu[3], zv[3], zuu[3], zvv[3], zuv[3];

	//Conversion of derivatives from XY to UV coordinates
	for (int i = 0; i < 3; i++)
	{
		TeTinVertexSet fderiv, sderiv;

		//(breakline_ == false) || ((breakline_ == true) &&
		//(this->vertex(vert[i]).type() != ONBREAKLINEVERTEX) &&
		//(this->vertex(vert[i]).type() != BREAKLINEFIRSTVERTEX) &&
		//(this->vertex(vert[i]).type() != BREAKLINELASTVERTEX))
		if ((((fbvertex_ == 0) && (breakline_ == false)) || ((fbvertex_ > 0) && (vert[i] < fbvertex_))) ||
			((breakline_ == true) && (this->vertex(vert[i]).type() != ONBREAKLINEVERTEX)))
		{
			v = vert[i];
			fderiv = vfderivSet_;
			sderiv = vsderivSet_;
		}
		else if (bside == 1)
		{
			v = vert[i] - fbvertex_;
			fderiv = vbrfderivSet_;
			sderiv = vbrsderivSet_;
		}
		else if (bside == 2)
		{
			v = vert[i] - fbvertex_;
			fderiv = vblfderivSet_;
			sderiv = vbrsderivSet_;
		}

		if (fderiv[v].location().y() >= TeMAXFLOAT)
		{
			zu[i] = 0.;
			zv[i] = 0.;
		}
		else
		{
			zu[i] = a * fderiv[v].location().x() +
					c * fderiv[v].location().y();
			zv[i] = b * fderiv[v].location().x() +
					d * fderiv[v].location().y();
		}

		if (sderiv[v].value() >= TeMAXFLOAT)
		{
			zuu[i] = 0.;
			zuv[i] = 0.;
			zvv[i] = 0.;
		}
		else
		{
			zuu[i] = aa * sderiv[v].location().x() +
					 2.*a*c * sderiv[v].value() +
					 cc * sderiv[v].location().y();
			zuv[i] = a*b * sderiv[v].location().x() +
					 (ad + bc) * sderiv[v].value() +
					 c*d * sderiv[v].location().y();
			zvv[i] = bb * sderiv[v].location().x() +
					 2.*b*d * sderiv[v].value() +
					 dd * sderiv[v].location().y();
		}
	}

	//Calculate U and V side sizes
	double lu = sqrt(aa + cc);
	double lv = sqrt(bb + dd);

	//Calculate angle between U and V axis and its cosine
	double thxu = atan2(c, a);
	double theta = atan2(d, b) - thxu;
	double cstheta = cos(theta);
	double lusntheta = lu * sin(theta);
	double lvsntheta = lv * sin(theta);

	//Calculate angle between U and S axis and its cosine
	double phi = atan2(d-c, b-a) - thxu;
	double thphi = theta - phi;

	//Coeficients of conversion from ST to UV coordinates when
	//S axis is paralel to v2v3 side
	double e = sin(thphi) / lusntheta;
	double f = -cos(thphi) / lusntheta;
	double g = sin(phi) / lvsntheta;
	double h = cos(phi) / lvsntheta;

	//Definition of the polynomial coefficients
	//Using (u,v) = (0,0) -> v1
	double p00 = vertSet[0].value();
	double p10 = zu[0];
	double p01 = zv[0];
	double p11 = zuv[0];
	double p20 = zuu[0] / 2.;
	double p02 = zvv[0] / 2.;

	//	Using (u,v) = (1,0) -> v2 and z(u,v), zu and zuu
	double h1 = vertSet[1].value() - p00 - p10 - p20;
	double h2 = zu[1] - p10 - zuu[0];
	double h3 = zuu[1] - zuu[0];
	double p30 = 10.*h1 - 4.*h2 + h3 / 2.;
	double p40 = -15.*h1 + 7.*h2 - h3;
	double p50 = 6.*h1 - 3.*h2 + h3 / 2.;

	//Using (u,v) = (0,1) -> v3 and z(u,v), zu and zuu
	h1 = vertSet[2].value() - p00 - p01 - p02;
	h2 = zv[2] - p01 - zvv[0];
	h3 = zvv[2] - zvv[0];
	double p03 = 10.*h1 - 4.*h2 + h3 / 2.;
	double p04 = -15.*h1 + 7.*h2 - h3;
	double p05 = 6.*h1 - 3.*h2 + h3 / 2.;

	double p41 = 5. * lv * cstheta * p50 / lu;
	double p14 = 5. * lu * cstheta * p05 / lv;

	//Using (u,v) = (1,0) -> v2 and z(u,v) and zuv
	h1 = zv[1] - p01 - p11 - p41;
	h2 = zuv[1] - p11 - 4.*p41;
	double p21 = 3.*h1 - h2;
	double p31 = -2.*h1 + h2;

	//Using (u,v) = (1,0) -> v3 and z(u,v) and zuv
	h1 = zu[2] - p10 - p11 - p14;
	h2 = zuv[2] - p11 - 4.*p14;
	double p12 = 3.*h1 - h2;
	double p13 = -2.*h1 + h2;

	//Using continuity restriction and zvv at v2 and zuu at v3
	double fg = f * g;
	double eh = e * h;
	double ee = e * e;
	double gg = g * g;

	double g1 = ee*g*(3.*fg + 2.*eh);
	double g2 = e*gg*(2.*fg + 3.*eh);

	h1 = -5.*ee*ee*f*p50 - 
		 ee*e*(fg + eh)*p41 -
		 gg*g*(fg + 4*eh) -
		 5.*gg*gg*h*p05;

	h2 = zvv[1]/2. - p02 - p12;
	h3 = zuu[2]/2. - p20 - p21;

	double p22 = (g1*h2 + g2*h3 - h1) / (g1 + g2);
	double p32 = h2 - p22;
	double p23 = h3 - p22;

	//Polynomial coefficients
	coef[0] = p00; coef[1] = p01; coef[2] = p02; coef[3] = p03; coef[4] = p04; coef[5] = p05;
	coef[6] = p10; coef[7] = p11; coef[8] = p12; coef[9] = p13; coef[10] = p14;
	coef[11] = p20; coef[12] = p21; coef[13] = p22; coef[14] = p23;
	coef[15] = p30; coef[16] = p31; coef[17] = p32;
	coef[18] = p40; coef[19] = p41;
	coef[20] = p50;

	//Coefficients of conversion from XY to UV coordinates
	coef[21] = ap;	coef[22] = bp;	coef[23] = cp;	coef[24] = dp;     
	coef[25] = vertSet[0].location().x();	coef[26] = vertSet[0].location().y();
}

double
TeTin::pointToSegmentDistance(TeCoord2D fseg, TeCoord2D lseg, TeCoord2D pt, TeCoord2D& pti)
{
	double ux = lseg.x() - fseg.x();
	double uy = lseg.y() - fseg.y();
	double vx = pt.x() - fseg.x();
	double vy = pt.y() - fseg.y();
	double uv = (ux*vx + uy*vy);

	if (uv < 0.)
	{
		//if (pti != NULL)
		pti.setXY(lseg.x(), lseg.y());
		
		return (sqrt (vx*vx + vy*vy));
	}
	else
	{
		ux = fseg.x() - lseg.x();
		uy = fseg.y() - lseg.y();
		vx = pt.x() - lseg.x();
		vy = pt.y() - lseg.y();
		uv = (ux*vx + uy*vy);

		if (uv < 0.)
		{
			//if (pti != NULL)
			pti.setXY(lseg.x(), lseg.y());

			return (sqrt (vx*vx + vy*vy));
		}
	}

	double a = lseg.y() - fseg.y();
	double b = fseg.x() - lseg.x();
	double c = lseg.x()*fseg.y() - fseg.x()*lseg.y();
	double aabb = a*a+b*b;

	double dist = fabs((a*pt.x()+b*pt.y()+c)/(sqrt(aabb)));
	
	//if (pti != NULL)
	//{
		double k = (b * pt.x()) - (a * pt.y());
		double x = ((b * k) - (a * c)) / aabb;
		double y = ((-a * k) - (b * c)) / aabb;
		pti.setXY(x, y);
	//}

	return dist;
}

bool
TeTin::checkTopology()
{
	for (long t = 1; t < triangleSet_.size() - 1; t++)
	{
		this->checkEdges(t);
	}

	return true;
}

void
TeTin::checkEdges(long t)
{
	if ((t == -1) || (t > (triangleSet_.size() - 1)))
		return;

	long e[3], v[3];
	triangleSet_[t].edges(e[0], e[1], e[2]);
	this->triangleVertices(t, v[0], v[1], v[2]);

	double a = vertexSet_[v[1]].location().x() - vertexSet_[v[0]].location().x();
	double b = vertexSet_[v[1]].location().y() - vertexSet_[v[0]].location().y();
	double c = vertexSet_[v[2]].location().x() - vertexSet_[v[0]].location().x();
	double d = vertexSet_[v[2]].location().y() - vertexSet_[v[0]].location().y();
	
	double s = (a * d) - (b * c);

	//To all triangle edges
	for (int i = 0; i < 3; i++)
	{
		//Make sure that triangle is inside
		if (s > 0.)
		{
			if (((edgeSet_[e[i]].from() == v[i]) &&
				(edgeSet_[e[i]].right() == t)) ||
				((edgeSet_[e[i]].to() == v[i]) &&
				(edgeSet_[e[i]].left() == t)))
			{
				edgeSet_[e[i]].swapTriangles();
			}
		}
		else
		{
			if (((edgeSet_[e[i]].to() == v[i]) &&
				(edgeSet_[e[i]].right() == t)) ||
				((edgeSet_[e[i]].from() == v[i]) &&
				(edgeSet_[e[i]].left() == t)))
			{
				edgeSet_[e[i]].swapTriangles();
			}
		}
	}
}

void
TeTin::orderEdges()
{
	//To all breaklines vertex
	for (long v = fbvertex_; v < (vertexSet_.size() - 1) - 1; v++)
	{
		//If vertex is last or sample point
		//if ((vertexSet_[i].type() > 4) ||
		//	(vertexSet_[i].type() == DELETEDVERTEX))
		//	continue;
		if ((vertexSet_[v].type() == BREAKLINELASTVERTEX) ||
			(vertexSet_[v].type() == NORMALVERTEX) ||
			(vertexSet_[v].type() == DELETEDVERTEX))
			continue;

		long v1 = v + 1;

		//If next vertex is last of a breakline
		if (vertexSet_[v1].type() == BREAKLINELASTVERTEX)
			continue;

		//Search edge from vertex to vertex+1
		long bedge = this->findEdge(v, v1);
		
		if (bedge == -1)
		{
			//if (vertexSet_[i].type() == 4)
			//	vertexSet_[i].type(6);
			//else
			//	vertexSet_[i].type(5);

			//vertexSet_[v1].type(4);
			//continue;
			if (vertexSet_[v].type() == BREAKLINEFIRSTVERTEX)
				vertexSet_[v].setType(NORMALVERTEX);
			else
				vertexSet_[v].setType(BREAKLINELASTVERTEX);

			vertexSet_[v1].setType(BREAKLINEFIRSTVERTEX);

			continue;
		}

		//Search edge from vertex+1 to vertex+2
		long v2 = v1 + 1;
		long nedge = this->findEdge(v1, v2);

		if (nedge == -1)
		{
			//vertexSet_[v1].type(5);

			//if (vertexSet_[v2].type() == 5)
			//	vertexSet_[v2].type(6);
			//else
			//	vertexSet_[v2].type(4);

			//continue;
			vertexSet_[v1].setType(BREAKLINELASTVERTEX);

			if (vertexSet_[v2].type() == BREAKLINELASTVERTEX)
				vertexSet_[v2].setType(NORMALVERTEX);
			else
				vertexSet_[v2].setType(BREAKLINEFIRSTVERTEX);

			continue;
		}

		//Make sure edge pointing to next vertex
		if (edgeSet_[bedge].to() == v)
		{
			edgeSet_[bedge].swapVertices();
			edgeSet_[bedge].swapTriangles();
		}

		long t = edgeSet_[bedge].right();
		long ledge = bedge;

		//Search vertex at right side
		while ((ledge != nedge) && (t != -1))
		{
			long e[3];
			triangleSet_[t].edges(e[0], e[1], e[2]);

			long j;
			for (j = 0; j < 3; j++)
			{
				if (e[j] == ledge)
					continue;

				if (edgeSet_[e[j]].to() == v1)
					break;

				if (edgeSet_[e[j]].from() == v1)
				{
					//Make edge pointing to next vertex
					edgeSet_[e[j]].swapVertices();
					edgeSet_[e[j]].swapTriangles();
					
					break;
				}
			}

			ledge = e[j];
			long nrt = edgeSet_[ledge].right();

			if (nrt == t)
				edgeSet_[ledge].swapTriangles();

			t = edgeSet_[ledge].right();
		}

		t = edgeSet_[bedge].left();

		ledge = bedge;

		//Search edges at left side
		while ((ledge != nedge) && (t != -1))
		{
			long e[3];
			triangleSet_[t].edges(e[0], e[1], e[2]);

			long j;
			for (j = 0; j < 3; j++)
			{
				if (e[j] == ledge)
					continue;

				if (edgeSet_[e[j]].from() == v1)
					break;
				
				if (edgeSet_[e[j]].to() == v1)
				{
					//Make edge pointing to opposite vertex of next vertex
					edgeSet_[e[j]].swapVertices();
					edgeSet_[e[j]].swapTriangles();

					break;
				}
			}

			ledge = e[j];
			long nrt = edgeSet_[ledge].right();

			if (nrt == t)
				edgeSet_[ledge].swapTriangles();
			
			t = edgeSet_[ledge].right();
		}
	}
}

long
TeTin::findEdge(long v1, long v2)
{
	long oldtri = 1;

	if ((oldtri < 1) || (oldtri >= (triangleSet_.size() - 1)))
		oldtri = 1;

	if (oldtri == 1)
	{
		for (long i = 1; i < triangleSet_.size() - 1; i++)
		{
			long e[3];
			triangleSet_[i].edges(e[0], e[1], e[2]);

			for (long m = 0; m < 3; m++)
			{
				if ((edgeSet_[e[m]].from() == v1) && (edgeSet_[e[m]].to() == v2))
				{
					oldtri = 1;
					return e[m];
				}

				if ((edgeSet_[e[m]].to() == v1) && (edgeSet_[e[m]].from()) == v2)
				{
					oldtri = 1;
					return e[m];
				}
			}
		}

		return -1;
	}

	long t = oldtri;
	long k = oldtri + 1;

	while ((t > 0) || (k < (triangleSet_.size() - 1)))
	{
		if (t > 0)
		{
			long e[3];
			triangleSet_[t].edges(e[0], e[1], e[2]);

			for (long m = 0; m < 3; m++)
			{
				if ((edgeSet_[e[m]].from() == v1) && (edgeSet_[e[m]].to() == v2))
				{
					oldtri = t;
					return e[m];
				}

				if ((edgeSet_[e[m]].to() == v1) && (edgeSet_[e[m]].from() == v2))
				{
					oldtri = t;
					return e[m];
				}
			}

			t--;
		}

		if (k < (triangleSet_.size() - 1))
		{
			long e[3];
			triangleSet_[k].edges(e[0], e[1], e[2]);

			for (long m = 0; m < 3; m++)
			{
				if ((edgeSet_[e[m]].from() == v1) && (edgeSet_[e[m]].to() == v2))
				{
					oldtri = k;
					return e[m];
				}

				if ((edgeSet_[e[m]].to() == v1) && (edgeSet_[e[m]].from() == v2))
				{
					oldtri = k;
					return e[m];
				}
			}

			t++;
		}
	}

	oldtri = 1;

	return -1;
}

bool
TeTin::onIsolineSegment(long e, TeTinVertex& vert)
{
	vert.geomId(0); //floating zvalue point
	TeCoord2D pt = vert.location();
	TeCoord2D pt1 = vertexSet_[edgeSet_[e].from()].location();
	TeCoord2D pt2 = vertexSet_[edgeSet_[e].to()].location();

	TeCoord2D pi;
	double dist = TePerpendicularDistance(pt, pt1, pt2, pi);
	if (dist < minedgesize_)
	{
	//if (this->onSegment(pt, pt1, pt2, minedgesize_))
	//{
		if (this->isIsolineSegment(e))
		{//If on isoline segment
			vert.value(vertexSet_[edgeSet_[e].from()].value());
			vert.geomId(1); //Fixed zvalue point

			return true;
		}
	//}
	}

	return false;
}

bool
TeTin::onSegment(TeCoord2D pt, TeCoord2D fseg, TeCoord2D lseg, double tol)
{
	if ((pt.x() < fseg.x()) && (pt.x() < lseg.x()))
		return false;

	if ((pt.y() < fseg.y()) && (pt.y() < lseg.y()))
		return false;

	if ((pt.x() > fseg.x()) && (pt.x() > lseg.x()))
		return false;

	if ((pt.y() > fseg.y()) && (pt.y() > lseg.y()))
		return false;

	double pax = pt.x() - fseg.x();
	double bax = lseg.x() - fseg.x();

	//Is on segment
	if ((pax == 0) && (bax == 0))
		return true;

	double pay = pt.y() - fseg.y();
	double bay = lseg.y() - fseg.y();

	if ((pay == 0) && (bay == 0))
		return true;

	double area2 = fabs((pay * bax) - (pax * bay));

	bax = fabs(bax);
	bay = fabs(bay);

	double minxy = bay;

	if (bax < bay)
		minxy = bax;

	double daux = area2 / (bax + bay - (minxy / 2));

	if (daux < tol)
		return true;

	return false;
}

bool
TeTin::isIsolineSegment(long e)
{
	//If sample point
	if ((vertexSet_[edgeSet_[e].to()].type() == NORMALVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == NORMALVERTEX))
		return false;

	//If breakline point
	if ((vertexSet_[edgeSet_[e].to()].type() == ONBREAKLINEVERTEX) ||
		(vertexSet_[edgeSet_[e].to()].type() == BREAKLINEFIRSTVERTEX) ||
		(vertexSet_[edgeSet_[e].to()].type() == BREAKLINELASTVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == ONBREAKLINEVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == BREAKLINEFIRSTVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == BREAKLINELASTVERTEX))
		return false;

	long vert0 = edgeSet_[e].from();
	long vert1;

	if (vert0 > edgeSet_[e].to())
	{
		vert0 = edgeSet_[e].to();
		vert1 = edgeSet_[e].from();
	}
	else
		vert1 = edgeSet_[e].to();

	//If first point is last point of line
	if (vertexSet_[vert0].type() == CONTOURLASTVERTEX)
		return false;

	//If last point is first point of line
	if (vertexSet_[vert1].type() == CONTOURFIRSTVERTEX)
		return false;

	vert0++;

	if (vert0 != vert1)
		return false;

	return true;
}

bool
TeTin::isBreaklineSegment(long e)
{
	//If sample point
	if ((vertexSet_[edgeSet_[e].to()].type() == NORMALVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == NORMALVERTEX))
		return false;

	//If breakline point
	if ((vertexSet_[edgeSet_[e].to()].type() == DELETEDVERTEX) ||
		(vertexSet_[edgeSet_[e].to()].type() == ONCONTOURVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == DELETEDVERTEX) ||
		(vertexSet_[edgeSet_[e].from()].type() == ONCONTOURVERTEX))
		return false;

	long v0 = edgeSet_[e].from();
	long v1;

	if (v0 > edgeSet_[e].to())
	{
		v0 = edgeSet_[e].to();
		v1 = edgeSet_[e].from();
	}
	else
		v1 = edgeSet_[e].to();

	//If first point is last point of line
	if (vertexSet_[v0].type() == BREAKLINELASTVERTEX)
		return false;

	//If last point is first point of line
	if (vertexSet_[v1].type() == BREAKLINEFIRSTVERTEX)
		return false;

	v0++;

	if (v0 != v1)
		return false;

	return true;
}

void
TeTin::writeTriangleSPRFile (std::string nameVert, std::string nameEdge, std::string nameTriang)
{
	fstream sprcfile;
	sprcfile.open ( nameVert.c_str(), ios::out );

	sprcfile << "Total de Vertices:" << vertexSet_.size()<<"\n"; 
	sprcfile << "INDICE X Y VALUE TYPE EDGE"<<"\n";
	for(unsigned int i=0;i < vertexSet_.size(); ++i)
	{
		std::string x = Te2String(vertexSet_[i].location().x(), 4);
		std::string y = Te2String(vertexSet_[i].location().y(), 4);
		std::string value;

		if (vertexSet_[i].value() >= TeMAXFLOAT)
			value = "TeMAXFLOAT";
		else
			value = Te2String(vertexSet_[i].value(), 4);

		sprcfile << i <<" "<< x<<" "
						   << y<<" "
						   << value<<" "
						   << vertexSet_[i].type()<<" "
						   << vertexSet_[i].oneEdge()<<"\n";
	}

	sprcfile.close();

	sprcfile.open(nameEdge.c_str(), ios::out);

	sprcfile << "Total de Arestas:" << edgeSet_.size()<<"\n"; 
	sprcfile << "INDICE FROM TO LEFT RIGHT TYPE"<<"\n";
	for(unsigned int i=0;i < edgeSet_.size(); ++i)
	{
		sprcfile << i <<"  "<< edgeSet_[i].from()<<"  "<<edgeSet_[i].to()<<"  "<<edgeSet_[i].left()<<"  "<<edgeSet_[i].right()<<"  "<<edgeSet_[i].type()<<"\n";
	}

	sprcfile.close();

	sprcfile.open(nameTriang.c_str(), ios::out);

	sprcfile << "Total de Triangulos:"<<triangleSet_.size()<<"\n";
	sprcfile << "INDICE FIRST SECOND THIRD"<<"\n";
	for (unsigned int i =0; i<triangleSet_.size(); ++i)
	{
		long e0, e1, e2;
		triangleSet_[i].edges(e0,e1,e2);
		sprcfile << i <<" "<< e0<<" "<< e1<<" "<< e2<<"\n";
	}

	sprcfile.close();
}

bool
TeTin::loadTIN(TeLayer* layer, TeDatabase* db)
{
	TeDatabasePortal* portal = db->getPortal();

	if (!portal)
	{
		delete portal;
		return false;
	}

	TeAttrTableVector attrTable;
	layer->getAttrTables(attrTable);

	std::string sql = "Select triangle_id, val1, val2, val3, type1, type2, type3, left1, right1, left2, right2, left3, right3 from " +
		attrTable[0].name() + " order by triangle_id";

	if (!portal->query(sql))
	{
		delete portal;
		return false;
	}

	int nrows = portal->numRows();

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Loading triangles...");
		TeProgress::instance()->setTotalSteps(nrows);
	}

	TeBox b;
	TeSAM::TeKdTree<TimKdTreeNode> indexPoint(b);
	TeSAM::TeRTree<long> indexEdge(b);
	
	int row = 0;
	while (portal->fetchRow())
	{
		if (TeProgress::instance())
			TeProgress::instance()->setProgress(row);

		if (TeProgress::instance()->wasCancelled())
		{
			TeProgress::instance()->reset();
			errorMessage_ = "Load triangles was cancelled.";
			delete portal;
			return false;
		}

		std::string triangId = Te2String(portal->getInt(0));
		double val1 = portal->getDouble(1);
		double val2 = portal->getDouble(2);
		double val3 = portal->getDouble(3);

		int type[3];
		type[0] = portal->getInt(4);
		type[1] = portal->getInt(5);
		type[2] = portal->getInt(6);

		long left1 = portal->getInt(7);
		long right1 = portal->getInt(8);
		long left2 = portal->getInt(9);
		long right2 = portal->getInt(10);
		long left3 = portal->getInt(11);
		long right3 = portal->getInt(12);

		std::string whereClause = "object_id = '" + triangId + "'";
		TePolygonSet triSet;
		layer->getPolygons(triSet, whereClause);

		TePolygon poly = triSet[0];
		TeCoord2D pt1 = poly[0][0];
		TeCoord2D pt2 = poly[0][1];
		TeCoord2D pt3 = poly[0][2];

	//Create triangle vertices
		TeTinVertex vert[3];
		vert[0].location().setXY(pt1.x(), pt1.y());
		vert[0].value(val1);

		vert[1].location().setXY(pt2.x(), pt2.y());
		vert[1].value(val2);

		vert[2].location().setXY(pt3.x(), pt3.y());
		vert[2].value(val3);

		//set vertex type
		for (int i = 0; i < 3; i++)
		{
			if (type[i] == -1)
				vert[i].setType(DELETEDVERTEX);
			else if (type[i] == 0)
				vert[i].setType(NORMALVERTEX);
			else if (type[i] == 1)
				vert[i].setType(ONCONTOURVERTEX);
			else if (type[i] == 2)
				vert[i].setType(ONBOXVERTEX);
			else if (type[i] == 3)
			{
				vert[i].setType(ONBREAKLINEVERTEX);
				breakline_ = true;
			}
			else if (type[i] == 4)
				vert[i].setType(CONTOURFIRSTVERTEX);
			else if (type[i] == 5)
				vert[i].setType(CONTOURLASTVERTEX);
			else if (type[i] == 6)
				vert[i].setType(BREAKLINEFIRSTVERTEX);
			else
				vert[i].setType(BREAKLINELASTVERTEX);
		}

		//set vertex box
		TeBox b0(vert[0].location(), vert[0].location());
		TeBox b1(vert[1].location(), vert[1].location());
		TeBox b2(vert[2].location(), vert[2].location());

		//insert vertex
		long indexV0 = vertexSet_.size();
		vector<TimKdTreeNode*> vecResult;
		indexPoint.search(b0, vecResult);
		if(vecResult.size() == 1)
		{
			indexV0 = vecResult[0]->getData();
		}
		else
		{
			indexPoint.insert(vert[0].location(), indexV0);
			vertexSet_.add(vert[0]);
		}

		long indexV1 = vertexSet_.size();
		vecResult = vector<TimKdTreeNode*>();
		indexPoint.search(b1, vecResult);
		if(vecResult.size() == 1)
		{
			indexV1 = vecResult[0]->getData();
		}
		else
		{
			indexPoint.insert(vert[1].location(), indexV1);
			vertexSet_.add(vert[1]);
		}
		
		long indexV2 = vertexSet_.size();
		vecResult = vector<TimKdTreeNode*>();
		indexPoint.search(b2, vecResult);
		if(vecResult.size() == 1)
		{
			indexV2 = vecResult[0]->getData();
		}
		else
		{
			indexPoint.insert(vert[2].location(), indexV2);
			vertexSet_.add(vert[2]);
		}

	//Create edges
		TeTinEdge edge[3];
		edge[0].setVertices(indexV0, indexV1);
		edge[1].setVertices(indexV1, indexV2);
		edge[2].setVertices(indexV2, indexV0);

		edge[0].setTriangles(left1, right1);
		edge[1].setTriangles(left2, right2);
		edge[2].setTriangles(left3, right3);

		//Set egde0 box
		TeBox boxE0;
		updateBox(boxE0, vertexSet_[indexV0].location());
		updateBox(boxE0, vertexSet_[indexV1].location());

		//Set egde1 box
		TeBox boxE1;
		updateBox(boxE1, vertexSet_[indexV1].location());
		updateBox(boxE1, vertexSet_[indexV2].location());

		//Set egde2 box
		TeBox boxE2;
		updateBox(boxE2, vertexSet_[indexV2].location());
		updateBox(boxE2, vertexSet_[indexV0].location());

		edge[0].setBox(boxE0);
		edge[1].setBox(boxE1);
		edge[2].setBox(boxE2);

		//set edge type
		if (((vert[0].type() == ONCONTOURVERTEX) || (vert[0].type() == CONTOURFIRSTVERTEX) || (vert[0].type() == CONTOURLASTVERTEX)) &&
			((vert[1].type() == ONCONTOURVERTEX) || (vert[1].type() == CONTOURFIRSTVERTEX) || (vert[1].type() == CONTOURLASTVERTEX)))
			edge[0].setType(ONCONTOUREDGE);
		if (((vert[0].type() == ONBREAKLINEVERTEX) || (vert[0].type() == BREAKLINEFIRSTVERTEX) || (vert[0].type() == BREAKLINELASTVERTEX)) &&
			((vert[1].type() == ONBREAKLINEVERTEX) || (vert[1].type() == BREAKLINEFIRSTVERTEX) || (vert[1].type() == BREAKLINELASTVERTEX)))
			edge[0].setType(ONBREAKLINEEDGE);
		else if ((vert[0].type() == ONBOXVERTEX) || (vert[1].type() == ONBOXVERTEX))
			edge[0].setType(DELETEDEDGE);
		else
			edge[0].setType(NORMALEDGE);

		if (((vert[1].type() == ONCONTOURVERTEX) || (vert[1].type() == CONTOURFIRSTVERTEX) || (vert[1].type() == CONTOURLASTVERTEX)) &&
			((vert[2].type() == ONCONTOURVERTEX) || (vert[2].type() == CONTOURFIRSTVERTEX) || (vert[2].type() == CONTOURLASTVERTEX)))
			edge[1].setType(ONCONTOUREDGE);
		if (((vert[1].type() == ONBREAKLINEVERTEX) || (vert[1].type() == BREAKLINEFIRSTVERTEX) || (vert[1].type() == BREAKLINELASTVERTEX)) &&
			((vert[2].type() == ONBREAKLINEVERTEX) || (vert[2].type() == BREAKLINEFIRSTVERTEX) || (vert[2].type() == BREAKLINELASTVERTEX)))
			edge[1].setType(ONBREAKLINEEDGE);
		else if ((vert[1].type() == ONBOXVERTEX) || (vert[2].type() == ONBOXVERTEX))
			edge[1].setType(DELETEDEDGE);
		else
			edge[1].setType(NORMALEDGE);

		if (((vert[2].type() == ONCONTOURVERTEX) || (vert[2].type() == CONTOURFIRSTVERTEX) || (vert[2].type() == CONTOURLASTVERTEX)) &&
			((vert[0].type() == ONCONTOURVERTEX) || (vert[0].type() == CONTOURFIRSTVERTEX) || (vert[0].type() == CONTOURLASTVERTEX)))
			edge[2].setType(ONCONTOUREDGE);
		if (((vert[2].type() == ONBREAKLINEVERTEX) || (vert[2].type() == BREAKLINEFIRSTVERTEX) || (vert[2].type() == BREAKLINELASTVERTEX)) &&
			((vert[0].type() == ONBREAKLINEVERTEX) || (vert[0].type() == BREAKLINEFIRSTVERTEX) || (vert[0].type() == BREAKLINELASTVERTEX)))
			edge[2].setType(ONBREAKLINEEDGE);
		else if ((vert[2].type() == ONBOXVERTEX) || (vert[0].type() == ONBOXVERTEX))
			edge[2].setType(DELETEDEDGE);
		else
			edge[2].setType(NORMALEDGE);
		
		//insert edges
		long indexE0 = edgeSet_.size();
		std::vector<long> vecResultEdge;
		if(indexEdge.search(edge[0].box(), vecResultEdge))
		{
			bool found = false;
			for(int i = 0; i < vecResultEdge.size(); ++i)
			{
				long idEdge = vecResultEdge[i];
				if(TeEquals(edge[0].box(), edgeSet_[idEdge].box()))
				{
					indexE0 = idEdge;
					found = true;
					break;
				}
			}

			if(found == false)
			{
				indexEdge.insert(edge[0].box(), indexE0);
				edgeSet_.add(edge[0]);
			}
		}
		else
		{
			indexEdge.insert(edge[0].box(), indexE0);
			edgeSet_.add(edge[0]);
		}

		long indexE1 = edgeSet_.size();
		vecResultEdge = std::vector<long>();
		if(indexEdge.search(edge[1].box(), vecResultEdge))
		{
			bool found = false;
			for(int i = 0; i < vecResultEdge.size(); ++i)
			{
				long idEdge = vecResultEdge[i];
				if(TeEquals(edge[1].box(), edgeSet_[idEdge].box()))
				{
					indexE1 = idEdge;
					found = true;
					break;
				}
			}

			if(found == false)
			{
				indexEdge.insert(edge[1].box(), indexE1);
				edgeSet_.add(edge[1]);
			}
		}
		else
		{
			indexEdge.insert(edge[1].box(), indexE1);
			edgeSet_.add(edge[1]);
		}

		long indexE2 = edgeSet_.size();
		vecResultEdge = std::vector<long>();
		if(indexEdge.search(edge[2].box(), vecResultEdge))
		{
			bool found = false;
			for(int i = 0; i < vecResultEdge.size(); ++i)
			{
				long idEdge = vecResultEdge[i];
				if(TeEquals(edge[2].box(), edgeSet_[idEdge].box()))
				{
					indexE2 = idEdge;
					found = true;
					break;
				}
			}

			if(found == false)
			{
				indexEdge.insert(edge[2].box(), indexE2);
				edgeSet_.add(edge[2]);
			}
		}
		else
		{
			indexEdge.insert(edge[2].box(), indexE2);
			edgeSet_.add(edge[2]);
		}


		vertexSet_[indexV0].setOneEdge(indexE0);
		vertexSet_[indexV1].setOneEdge(indexE1);
		vertexSet_[indexV2].setOneEdge(indexE2);

	//Create triangle
		TeTinTriangle triang;
		triang.setEdges(indexE0, indexE1, indexE2);	
		triang.setBox(poly.box());

		long indexT = triangleSet_.size();
		indexTriang_->insert(triang.box(), indexT);
		triangleSet_.add(triang);

		row++;
	}

	if (TeProgress::instance())
		TeProgress::instance()->reset();

	return true;
}

bool
TeTin::quinticInterpolation(TeRaster*& grid, int interpolator)
{
	if (interpolator == 2)//Quintic Surface Adjust without breaklines
	{
		fbvertex_ = 0;
		breakline_ = false;
	}

	this->vertexDerivatives();

	double dummy = grid->params().dummy_[0];

	double resx = grid->params().resx_;
	double resy = grid->params().resy_;

	double resx1 = grid->box().x1() + (resx / 2.);
	double resy2 = grid->box().y2() - (resy / 2.);

	int ncols = grid->params().ncols_;
	int nlines = grid->params().nlines_;

	if (TeProgress::instance())
	{
		TeProgress::instance()->setCaption("TIN - Triangular Irregular Network");
		TeProgress::instance()->setMessage("Creating MNT...");
		TeProgress::instance()->setTotalSteps(nlines);
	}

	for (int line = 0; line < nlines; line++)
	{
		if (TeProgress::instance())
			TeProgress::instance()->setProgress(line);

		for (int col = 0; col < ncols; col++)
		{
			TeCoord2D pt;
			pt.setXY((resx1 + ((double)col * resx)), (resy2 - ((double)line * resy)));

			TeBox boxPt(pt, pt);
			long t = this->triangleAt(boxPt);

			if (t == -1)
			{
				grid->setElement(col, line, dummy);
				continue;
			}

			long vert[3];
			this->triangleVertices(t, vert[0], vert[1], vert[2]);

			TeTinVertex vert0 = vertexSet_[vert[0]];
			TeTinVertex vert1 = vertexSet_[vert[1]];
			TeTinVertex vert2 = vertexSet_[vert[2]];

			if ((vert0.value() >= TeMAXFLOAT) || (vert1.value() >= TeMAXFLOAT) || (vert2.value() >= TeMAXFLOAT))
			{
				grid->setElement(col, line, dummy);
				continue;
			}

			long neigh[3];
			this->triangle3Neighbors(t, neigh[0], neigh[1], neigh[2]);

			long i;
			for (i = 0; i < 3; i++)
			{
				if (neigh[i] == -1)
					break;
			}

			TeTinVertexSet vertSet;
			vertSet.add(vert0);
			vertSet.add(vert1);
			vertSet.add(vert2);

			double coef[27];
			this->defineAkimaCoeficients(t, vertSet, coef);

			double ap, bp, cp, dp, x0, y0, p00, p01, p02, p03, p04, p05,
			p10, p11, p12, p13, p14, p20, p21, p22, p23, p30, p31, p32,
			p40, p41, p50;

			//	Polynomial coefficients
			p00 = coef[0]; p01 = coef[1]; p02 = coef[2]; p03 = coef[3]; p04 = coef[4]; p05 = coef[5];
			p10 = coef[6]; p11 = coef[7]; p12 = coef[8]; p13 = coef[9]; p14 = coef[10];
			p20 = coef[11]; p21 = coef[12]; p22 = coef[13]; p23 = coef[14];
			p30 = coef[15]; p31 = coef[16]; p32 = coef[17];
			p40 = coef[18]; p41 = coef[19];
			p50 = coef[20];

			//	Coefficients of conversion from XY to UV coordinates
			ap = coef[21]; bp = coef[22]; cp = coef[23]; dp = coef[24];     
			x0 = coef[25]; y0 = coef[26];

			//Converts point from XY to UV
			double u = ap*(pt.x() - x0) +
				bp*(pt.y() - y0);
			double v = cp*(pt.x() - x0) +
				dp*(pt.y() - y0);

			//Evaluates the polynomial
			double p0 = p00 + v*(p01 + v*(p02 + v*(p03 + v*(p04 + v*p05))));
			double p1 = p10 + v*(p11 + v*(p12 + v*(p13 + v*p14)));
			double p2 = p20 + v*(p21 + v*(p22 + v*p23));
			double p3 = p30 + v*(p31 + v*p32);
			double p4 = p40 + v*p41;

			double zvalue = (p0 + u*(p1 + u*(p2 + u*(p3 + u*(p4 + u*p50)))));

			if (zvalue > TeTinBIGFLOAT)
				grid->setElement(col, line, dummy);
			else
				grid->setElement(col, line, zvalue);

			if (TeProgress::instance()->wasCancelled())
			{
				TeProgress::instance()->reset();
				errorMessage_ = "Regular grid creation was cancelled.";
				return false;
			}
		}
	}

	if (TeProgress::instance())
		TeProgress::instance()->reset();

	return true;
}

void
TeTin::point3dListFilter(TeTinVertexSet vertSet, double tol)
{
	double vectd[200], vectz[200], vectx[200], vecty[200], fvectz[200];
	double x0, y0, z0;
	int npts = 0;
	int	ptype[200];
	int maxdiffindex = 0;
	TeTinVertex vertaux[200];
	int i = 0;

	TeTinVertexSet::iterator itVert = vertSet.begin();
	while (itVert != vertSet.end())
	{
		if ((itVert->location().x() >= TeMAXFLOAT) || (npts > 199))
		{//if last point of a line
			double maxdiff = TeMAXFLOAT;
			double maxfixptdiff = TeMAXFLOAT;

			int j = 4;

			while ((maxdiff > tol) && (maxfixptdiff > tol) && (j > 3) && (npts > 3))
			{
				j = 0;

				for (i = 0; i < npts; i++)
				{
					if (ptype[i] == 0)
						continue;

					vectx[j] = vectd[i];
					vecty[j++] = vectz[i];
				}

				if (j < 3)
					break;

				int degree;

				if (j < 5)
					degree = 1;
				else if (j < 8)
					degree = 2;
				else
					degree = 3;

				double coef[5];
				this->leastSquareFitting(vectx, vecty, j, degree, coef);

				for (i = 0; i < npts; i++)
				{
					fvectz[i] = coef[0];

					for (j = 1; j <= degree; j++)
						fvectz[i] += coef[j]*pow(vectd[i], (double)j);
				}

				maxfixptdiff = 0;

				for (i = 0; i < npts; i++)
				{
					if ((ptype[i] == 0) || (ptype[i] == 1))
						continue;

					if (fabs(fvectz[i] - vectz[i]) > maxfixptdiff)
						maxfixptdiff = fabs(fvectz[i] - vectz[i]);
				}

				maxdiff = 0;

				for (i = 0; i < npts; i++)
				{
					if ((ptype[i] == 0) || (ptype[i] == 2))
						continue;

					if (fabs(fvectz[i]-vectz[i]) > maxdiff)
					{
						maxdiff = fabs(fvectz[i] - vectz[i]);
						maxdiffindex = i;
					}
				}

				if (i == 0)
					break;

				ptype[maxdiffindex] = 0;
			}

			for (i = 0; i < npts; i++)
			{
				*itVert = vertaux[i];
				itVert->value(fvectz[i]);
				z0 = itVert->value();
			}

			npts = 0;
			continue;
		}

		if (npts == 0)
		{
			x0 = itVert->location().x();
			y0 = itVert->location().y();
		}

		vertaux[npts] = *itVert;
		vectd[npts] = sqrt((itVert->location().x() - x0)*(itVert->location().x() - x0) +
			(itVert->location().y() - y0)*(itVert->location().y() - y0));
		
		if (npts > 0)
			vectd[npts] += vectd[npts-1];

		if (itVert->value() < TeMAXFLOAT)
			ptype[npts] = 1;
		else
			ptype[npts] = 0;

		if (itVert->geomId() == 1)
			ptype[npts] = 2;

		vectz[npts] = itVert->value();
		fvectz[npts++] = itVert->value();

		x0 = itVert->location().x();
		y0 = itVert->location().y();

		itVert++;
	}
}

bool
TeTin::leastSquareFitting(double* vectx, double* vecty, int np, int degree, double* coef)
{
	int i, j, m, n;
	double** matx = new double*[6];
	double powx[200], sumpow[200], fx[200];

	if (np > 200)
		return false;

	for (i = 0; i < 6; i++)
		matx[i] = new double[6];

	for (i = 0; i < np; i++)
	{
		powx[i] = 1.;
		fx[i] = vecty[i];
	}

	//Compute the rigth hand side of the first normal equation 
	matx[0][degree+1] = 0;
	for (i = 0; i < np; i++)
		matx[0][degree+1] = matx[0][degree+1] + vecty[i];

	//Compute the rigth hand side of the other normal equation and also the sums 	
	sumpow[0]= np;
	for (i = 1; i <= degree; i++)
	{
		sumpow[i] = 0;
		matx[i][degree+1] = 0;

		for (j = 0; j < np; j++)
		{
			powx[j] = powx[j] * vectx[j];
			fx[j] = fx[j] * vectx[j];
			sumpow[i] = sumpow[i] + powx[j];
			matx[i][degree+1] = matx[i][degree+1] + fx[j];
		}
	}

	for (i = degree + 1; i <( 2 * (degree+1)); i++)
	{
		sumpow[i] = 0;
		for (j = 0; j < np; j++)
		{
			powx[j] = powx[j] * vectx[j];
			sumpow[i] = sumpow[i] + powx[j];
		}
	}

	for (i = 0 ; i <= degree; i++)
		for (j = 0; j <= degree; j++)
			matx[i][j] = sumpow[i+j];

	//Perform Gaussian elimination
	m = degree + 1;
	n = 1;

	if (!this->gaussElimination(m,n,matx))
	{
		for (i = 0; i < 6; i++)
		{
			delete [] matx[i];
			matx[i] = NULL;
		}

		return false;
	}

	for (i = 0; i < (degree + 1); i++)
		coef[i] = matx[i][m];

	for (i = 0; i < 6; i++)
	{
		delete [] matx[i];
		matx[i] = NULL;
	}

	delete matx;

	return true;
}

bool
TeTin::gaussElimination(int nvars, int neqs, double** matrix)
{
	double eps = 0.000000001; //Estimated error bound (machine epsilon)

	if (nvars > 1)
	{
		for (int i = 0; i < (nvars - 1); i++)
		{
			double pivot = fabs(matrix[i][i]);

			int ii = i + 1;
			int jj = i;

			//Search for the index in of maximum pivot value
			for (int j = ii; j < nvars; j++)
			{
				if (fabs(matrix[j][i]) > pivot)
				{
					pivot = fabs(matrix[j][i]);
					jj = j;
				}
			}

			if (i != jj)
			{
				//Interchange rows i and index jj
				for (int j = i; j < (nvars + neqs); j++)
				{
					double aux = matrix[i][j];
					matrix[i][j] = matrix[jj][j];
					matrix[jj][j] = aux;
				}
			}

			//Check if pivot is too smal
			if (pivot < eps)
			{
				errorMessage_ = "Singular matrix!";
				return false;
			}

			//Forward elimination step
			for (int j = ii; j < nvars; j++)
			{
				for (int k = ii; k < (nvars + neqs); k++)
					matrix[j][k] = (matrix[j][k] - ((matrix[j][i] * matrix[i][k]) / matrix[i][i]));
			}
		}

		if (fabs(matrix[nvars - 1][nvars - 1]) < eps)
		{
			errorMessage_ = "Singular matrix!";
			return false;
		}

		//Back substitution
		for (int i = 0; i < neqs; i++)
		{
			matrix[nvars - 1][i + nvars] = matrix[nvars - 1][i + nvars] / matrix[nvars - 1][nvars - 1];

			for (int j = 0; j < (nvars - 1); j++)
			{
				int k = nvars - j - 2;
				int kk = k + 1;

				for (int l = kk; l < nvars; l++)
					matrix[k][i + nvars] = matrix[k][i + nvars] - (matrix[l][i + nvars] * matrix[k][l]);

				matrix[k][i + nvars] = matrix[k][i + nvars] / matrix[k][k];
			}
		}
	}
	else
	{
		if (fabs(matrix[0][0]) < eps)
		{
			errorMessage_ = "Singular matrix!";
			return false;
		}

		for (int i = 0; i < neqs; i++)
			matrix[0][nvars + i] = matrix[0][nvars + i] / matrix[0][0];
	}

	return true;
}

bool
TeTin::isNeighborOnIsoOrBreakline(long t, long nviz)
{
	long e = triangleSet_[t].edgeAt(nviz);

	if (this->isIsolineSegment(e))
		return true;

	if (this->isBreaklineSegment(e))
		return true;

	return false;
}

void
TeTin::generateContour(double contour, TeContourLine*& ptline)
{
	for (long t = 1; t < (triangleSet_.size() - 1); t++)
	{
		long verts[3];
		this->triangleVertices(t, verts[0], verts[1], verts[2]);

		TeTinVertex vert[3];
		for (int j = 0; j < 3; j++)
		{
			vert[j].location().x(vertexSet_[verts[j]].location().x());
			vert[j].location().y(vertexSet_[verts[j]].location().y());
			vert[j].value(vertexSet_[verts[j]].value());
		}

		//Test the vertex values
		for (int j = 0; j < 3; j++)
		{
			if (vert[j].value() == contour)
				vert[j].value(contour * 1.00001);
		}

		if ((vert[0].value() == vert[1].value()) && (vert[0].value() == vert[2].value()))
			continue;

		if ((vert[0].value() >= TeMAXFLOAT) || (vert[1].value() >= TeMAXFLOAT) || (vert[2].value() >= TeMAXFLOAT))
			continue;

		if ((vert[0].value() < contour) && (vert[1].value() < contour) && (vert[2].value() < contour))
			continue;

		if ((vert[0].value() > contour) && (vert[1].value() > contour) && (vert[2].value() > contour))
			continue;

		bool flag = true;

		for (int j = 0; j < 3; j++)
		{
			if ((vert[j].value() == contour) && (vert[(j+1)%3].value() > contour) && (vert[(j+2)%3].value() > contour))
			{
				flag = false;
				break;
			}

			if ((vert[j].value() == contour) && (vert[(j+1)%3].value() < contour) && (vert[(j+2)%3].value() < contour))
			{
				flag = false;
				break;
			}
		}

		if (flag == false)
			continue;

		//Define intersection between contour and triangle
		this->defineInterTriangle(ptline, vert);
	}
}

bool
TeTin::defineInterTriangle(TeContourLine*& ptline, TeTinVertex* vert)
{
	int contrl = 0;

	//Verify if contour intercepts triangle
	double contour = ptline->value();

	if ((contour < vert[0].value()) && (contour < vert[1].value()) && (contour < vert[2].value()))
		return false;

	if ((contour > vert[0].value()) && (contour > vert[1].value()) && (contour > vert[2].value()))
		return false;

	if (((contour < vert[0].value()) && (contour > vert[1].value())) ||
		((contour > vert[0].value()) && (contour < vert[1].value())))
	{
		//Calculate intersection with edge 0
		if (!this->defineInterEdge(ptline, vert[0], vert[1]))
			return false;

		contrl++;
	}

	if (((contour < vert[1].value()) && (contour > vert[2].value())) ||
		((contour > vert[1].value()) && (contour < vert[2].value())))
	{
		//Calculate intersection with edge 1
		if (!this->defineInterEdge(ptline, vert[1], vert[2]))
			return false;

		contrl++;
	}

	if (((contour < vert[2].value()) && (contour > vert[0].value())) ||
		((contour > vert[2].value()) && (contour < vert[0].value())))
	{
		//Calculate intersection with edge 2
		if (!this->defineInterEdge(ptline, vert[2], vert[0]))
			return false;

		contrl++;
	}

	if (contrl != 2)
		return false;

	return true;
}

bool
TeTin::defineInterEdge(TeContourLine*& ptline, TeTinVertex fpt, TeTinVertex spt)
{
	if (spt.value() == fpt.value())
		return false;

	TeTinVertex vert;
	vert.location().x(spt.location().x() - fpt.location().x());
	vert.location().y(spt.location().y() - fpt.location().y());
	vert.value(spt.value() - fpt.value());

	double paux = (ptline->value() - fpt.value()) / vert.value();

	TeCoord2D pt;
	pt.x(fpt.location().x() + vert.location().x() * paux);
	pt.y(fpt.location().y() + vert.location().y() * paux);

	ptline->add(pt);

	return true;
}

long
TeTin::onSameSide(TeCoord2D pt1, TeCoord2D pt2, TeCoord2D fseg, TeCoord2D lseg)
{
	double a = lseg.y() - fseg.y();
	double b = fseg.x() - lseg.x();
	double c = lseg.x()*fseg.y() - fseg.x()*lseg.y();
	double ip = a*pt1.x() + b*pt1.y() + c;

	if (ip == 0) //On segment
		return -1;

	double ipt = a*pt2.x() + b*pt2.y() + c;

	if ((ip > 0) && (ipt < 0))
		return 0;

	if ((ip < 0) && (ipt > 0))
		return 0;

	//On same side
	return 1;
}