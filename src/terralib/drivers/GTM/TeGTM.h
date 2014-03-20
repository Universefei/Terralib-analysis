/************************************************************************************
TerraLib - a library for developing GIS applications.
Copyright © 2001-2007 INPE and Tecgraf/PUC-Rio.

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
/*! \file TeGTM.h
    \brief This file contains classes for .gtm support in TerraLib develop by the Exercito Brasileiro
*/
#ifndef  __TERRALIB_INTERNAL_GTMDRIVER_H
#define  __TERRALIB_INTERNAL_GTMDRIVER_H

#include <cstdio>
#include <vector>
#include <TeProjection.h>
#include <TeDatum.h>
#include <TeLayer.h>
#include <TeDatabase.h>
#include <TeAttribute.h>

#include <tdkebdgnimport.h>

using namespace std;

//! TeGTMHeader .gtm files header.
/*!
	This class contains the access methods for the .gtm header attributes.
	Attributes used in this class:
	version - Integer Version of GTM file – Place 211
	code - String*10 Fixed-length String – Place TrackMaker
	wli - Byte Draws thick lines on Maps – Set to 0
	vwt - Byte Reservation – Set to 0
	gradnum - Byte Font size of the Grid Lines – Default = 8
	wptnum - Byte Reservation – Set to 0
	usernum - Byte Reservation – Set to 0
	coriml - Byte Icon color
	ndatum - Byte Reservation – Set to 0
	gradcolor - Long Grid color – RGB – Default = 0 (Black)
	bcolor - Long Background color– RGB – Default = 16777215 (White)
	nwptstyles - Long Number of Waypoint Styles	Defines the number of times that the stylefont1 sequence will be repeated.
	usercolor - Long Text Color of default Waypoint – RGB – Default = 0 (Black)
	nwpts - Long Number of Waypoints
	Defines - the number of times that the Wpts1 sequence will be repeated
	ntrcks - Long Number of Trackpoints	Defines the number of times that the trcks1 sequence will be repeated
	nrtes - Long Number of RoutePoints	Defines the number of times that the rtes1 sequence will be repeated

	maxlon -
	minlon - 	Single Bounding Box that stores the actual extent of the data in the file. Represents the minimum-bounding
	maxlat -	rectangle orthogonal to the X and Y axes that contains all shapes. Basically used by the Catalog of Images.
	minlat -

	n_maps - Long Number of Map images
	n_tk - Long Number of Tracklog Styles
	rectangular - Boolean Activates Rectangular coordinates when GTM file is opened – Default = false
	truegrid - Boolean Activates True Grid Mode when GTM file is opened – Default = False
	LabelColor - Boolean Text Color of Tracklog Labels
	wpttachado - Boolean Reservation – Set to False
	usernegrit - Boolean Reservation – Set to False
	useritalic - Boolean Reservation – Set to False
	usersublin - Boolean Reservation – Set to False
	usertachado - Boolean Reservation – Set to False
	map - Boolean Set to TRUE if there are maps
	LabelSize - Boolean Font Size of Tracklog Labels
	gradfont - String Font Name of the Grid Text – Default = Times New Roman
	LabelFont - String Font Name of Tracklogs Labels – Default = Times New Roman
	userfont - String Reservation – No Text
	newdatum - String Reservation – No Text
    \author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMHeader
{
private:
	short version_, labelsize_;
	char* code_;//code deve ser char[10]
	unsigned char wli_, vwt_, gradnum_, wptnum_, usernum_, coriml_, ndatum_;
	long gradcolor_, bcolor_, nwptstyles_, usercolor_, nwpts_, ntrcks_, nrtes_, n_maps_, n_tk_, labelcolor_;
	float maxlon_, minlon_, maxlat_, minlat_, layers_, iconnum_;
	char *gradfont_, *labelfont_, *userfont_, *newdatum_;
	bool rectangular_, truegrid_, usernegrit_, useritalic_, usersublin_, usertachado_, map_;
public:
	//! Constructor
	TeGTMHeader();

	//! Destructor
	~TeGTMHeader();

	//! Header reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMHeader(FILE* file);

	//! Header writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMHeader(FILE* file);

	//! version_ attribute Getter
	short getVersion(){return version_;};

	//! version_ attribute Setter
    /*!
      \param GTM file version to be set.
    */
	void setVersion(short version){version_ = version;};

	//! labelsize_ attribute Getter
	short getLabelsize(){return labelsize_;};

	//! labelsize_ attribute Setter
    /*!
      \param GTM file labelsize to be set.
    */
	void setLabelsize(short labelsize){labelsize_ = labelsize;};

	//! code_ attribute Getter
	char* getCode(){return code_;};

	//! code_ attribute Setter
    /*!
      \param GTM file code to be set, place Trackmaker.
    */
	void setCode(char* code){code_ = code;};

	//! gradfont_ attribute Getter
	char* getGradfont(){return gradfont_;};

	//! gradfont_ attribute Setter
    /*!
      \param GTM file grad font to be set, Default = Times New Roman.
    */
	void setGradfont(char* gradfont){gradfont_ = gradfont;};

	//! userfont_ attribute Getter
	char* getlabelFont(){return labelfont_;};

	//! userfont_ attribute Setter
    /*!
      \param GTM file label font to be set, Default = Times New Roman.
    */
	void setlabelFont(char* labelfont){labelfont_ = labelfont;};

	//! attribute Getter
	char* getUserfont(){return userfont_;};

	//! attribute Setter
    /*!
      \param GTM file user font to be set, String Reservation – No Text.
    */
	void setUserfont(char* userfont){userfont_ = userfont;};

	//! newdatum_attribute Getter
	char* getNewdatum(){return newdatum_;};

	//! newdatum_attribute Setter
    /*!
      \param GTM file new datum to be set, String Reservation – No Text.
    */
	void setNewdatum(char* newdatum){newdatum_ = newdatum;};

	//! wli_ attribute Getter
	unsigned char getWli(){return wli_;};

	//! wli_ attribute Setter
    /*!
      \param GTM file wli to be set, Set to 0.
    */
	void setWli(unsigned char wli){wli_ = wli;};

	//! vwt_ attribute Getter
	unsigned char getVwt(){return vwt_;};

	//! vwt_ attribute Setter
    /*!
      \param GTM file vwt to be set, Set to 0.
    */
	void setVwt(unsigned char vwt){vwt_ = vwt;};

	//! gradnum_ attribute Getter
	unsigned char getGradnum(){return gradnum_;};

	//! gradnum_ attribute Setter
    /*!
      \param GTM file gradnum to be set, Default = 8.
    */
	void setGradnum(unsigned char gradnum){gradnum_ = gradnum;};

	//! wptnum_ attribute Getter
	unsigned char getWptnum(){return wptnum_;};

	//! wptnum_ attribute Setter
    /*!
      \param GTM file wptnum to be set, Set to 0.
    */
	void setWptnum(unsigned char wptnum){wptnum_ = wptnum;};

	//! usernum_ attribute Getter
	unsigned char getUsernum(){return usernum_;};

	//! usernum_ attribute Setter
    /*!
      \param GTM file usernum to be set, Set to 0.
    */
	void setUsernum(unsigned char usernum){usernum_ = usernum;};

	//! coriml_ attribute Getter
	unsigned char getCoriml(){return coriml_;};

	//! coriml_ attribute Setter
    /*!
      \param GTM file coriml to be set, Byte Icon color.
    */
	void setCoriml(unsigned char coriml){coriml_ = coriml;};

	//! ndatum_ attribute Getter
	unsigned char getNdatum(){return ndatum_;};

	//! ndatum_ attribute Setter
    /*!
      \param GTM file ndatum to be set, Set to 0.
    */
	void setNdatum(unsigned char ndatum){ndatum_ = ndatum;};

	//! gradcolor_ attribute Getter
	long getGradcolor(){return gradcolor_;};

	//! gradcolor_ attribute Setter
    /*!
      \param GTM file gradcolor to be set, Default = 0.
    */
	void setGradcolor(long gradcolor){gradcolor_ = gradcolor;};

	//! bcolor_ attribute Getter
	long getBcolor(){return bcolor_;};

	//! bcolor_ attribute Setter
    /*!
      \param GTM file bcolor to be set, Default = 16777215.
    */
	void setBcolor(long bcolor){bcolor_ = bcolor;};

	//! usercolor_ attribute Getter
	long getNwptstyles(){return nwptstyles_;};

	//! usercolor_ attribute Setter
    /*!
      \param GTM file nwptstyles to be set.
    */
	void setNwptstyles(long nwptstyles){nwptstyles_ = nwptstyles;};

	//! attribute Getter
	long getUsercolor(){return usercolor_;};

	//! attribute Setter
    /*!
      \param GTM file usercolor to be set, Default = 0.
    */
	void setUsercolor(long usercolor){usercolor_ = usercolor;};

	//! nwpts_ attribute Getter
	long getNwpts(){return nwpts_;};

	//! nwpts_ attribute Setter
    /*!
      \param GTM file nwpts to be set.
    */
	void setNwpts(long nwpts){nwpts_ = nwpts;};

	//! ntrcks_ attribute Getter
	long getNtracks(){return ntrcks_;};

	//! ntrcks_ attribute Setter
    /*!
      \param GTM file ndatum to be set.
    */
	void setNtracks(long ntrcks){ntrcks_ = ntrcks;};

	//! nrtes_ attribute Getter
	long getNrtes(){return nrtes_;};

	//! nrtes_ attribute Setter
    /*!
      \param GTM file nrtes to be set.
    */
	void setNrtes(long nrtes){nrtes_ = nrtes;};

	//! n_maps_ attribute Getter
	long getN_maps(){return n_maps_;};

	//! n_maps_ attribute Setter
    /*!
      \param GTM file n_maps_ to be set.
    */
	void setN_maps(long n_maps){n_maps_ = n_maps;};

	//! n_tk_ attribute Getter
	long getN_tk(){return n_tk_;};
	//! n_tk_ attribute Setter
    /*!
      \param GTM file n_tk to be set.
    */
	void setN_tk(long n_tk){n_tk_ = n_tk;};

	//! labelcolor_ attribute Getter
	long getLabelcolor(){return labelcolor_;};
	//! labelcolor_ attribute Setter
    /*!
      \param GTM file labelcolor to be set.
    */
	void setLabelcolor(long labelcolor){labelcolor_ = labelcolor;};

	//! maxlon_ attribute Getter
	float getMaxlon(){return maxlon_;};

	//! maxlon_ attribute Setter
    /*!
      \param GTM file maxlon to be set.
    */
	void setMaxlon(float maxlon){maxlon_ = maxlon;};

	//! minlon_ attribute Getter
	float getMinlon(){return minlon_;};

	//! minlon_ attribute Setter
    /*!
      \param GTM file minlon to be set.
    */
	void setMinlon(float minlon){minlon_ = minlon;};

	//! maxlat_ attribute Getter
	float getMaxlat(){return maxlat_;};

	//! maxlat_ attribute Setter
    /*!
      \param GTM file maxlat to be set.
    */
	void setMaxlat(float maxlat){maxlat_ = maxlat;};

	//! minlat_ attribute Getter
	float getMinlat(){return minlat_;};

	//! minlat_ attribute Setter
    /*!
      \param GTM file minlat to be set.
    */
	void setMinlat(float minlat){minlat_ = minlat;};

	//! layers_ attribute Getter
	float getLayers(){return layers_;};

	//! layers_ attribute Setter
    /*!
      \param GTM file layers to be set.
    */
	void setLayers(float layers){layers_ = layers;};

	//! iconnum_ attribute Getter
	float getIconnum(){return iconnum_;};

	//! iconnum_ attribute Setter
    /*!
      \param GTM file iconnum to be set.
    */
	void setIconnum(float iconnum){iconnum_ = iconnum;};

	//! rectangular_ attribute Getter
	bool getRectangular(){return rectangular_;};

	//! rectangular_ attribute Setter
    /*!
      \param GTM file rectangular to be set.
    */
	void setRectangular(bool rectangular){rectangular_ = rectangular;};

	//! truegrid_ attribute Getter
	bool getTruegrid(){return truegrid_;};

	//! truegrid_ attribute Setter
    /*!
      \param GTM file truegrid to be set.
    */
	void setTruegrid(bool truegrid){truegrid_ = truegrid;};

	//! usernegrit_ attribute Getter
	bool getUsernegrit(){return usernegrit_;};

	//! usernegrit_ attribute Setter
    /*!
      \param GTM file usernegrit to be set.
    */
	void setUsernegrit(bool usernegrit){usernegrit_ = usernegrit;};

	//! useritalic_ attribute Getter
	bool getUseritalic(){return useritalic_;};

	//! useritalic_ attribute Setter
    /*!
      \param GTM file useritalic to be set.
    */
	void setUseritalic(bool useritalic){useritalic_ = useritalic;};

	//! usersublin_ attribute Getter
	bool getUsersublin(){return usersublin_;};

	//! usersublin_ attribute Setter
    /*!
      \param GTM file usersublin to be set.
    */
	void setUsersublin(bool usersublin){usersublin_ = usersublin;};

	//! usertachado_ attribute Getter
	bool getUsertachado(){return usertachado_;};

	//! usertachado_ attribute Setter
    /*!
      \param GTM file usertachado to be set.
    */
	void setUsertachado(bool usertachado){usertachado_ = usertachado;};

	//! map_ attribute Getter
	bool getMap(){return map_;};

	//! map_ attribute Setter
    /*!
      \param GTM file map to be set.
    */
	void setMap(bool map){map_ = map;};
};


//! TeGTMGridDatum class with projection/datum informations.
/*!
	Attributes used
	Ngrid - Integer Number of the predefined grid. See the table below.
	Origin - Double User Grid - Longitude of origin
	falseeast - Double User Grid – False Easting
	scale1 - Double User Grid – Scale Factor
	falsenorthing - Double User Grid – False Northing (Set to 0)
	ndatum - Integer Number of Predefined Datum, which the data were stored in GTM File.
	axis - Double User Datum – Major Semi-Axe of the Earth (Set to 0)
	flattenig - Double User Datum – Flattening of the Earth (Set to 0)
	dx - Integer User Datum – Delta X (Set to 0)
	dy - Integer User Datum – Delta Y (Set to 0)
	dz - Integer User Datum – Delta Z (Set to 0)    
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMGridDatum
{
private:
	short ngrid_, ndatum_, dx_, dy_, dz_;
	double origin_, falseeast_, scale1_, falsenorthing_, axis_, flattening_;
public:
	//! Constructor
	TeGTMGridDatum();

	//! Destructor
	~TeGTMGridDatum();

	//! GridDatum reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMGridDatum(FILE* file);

	//! GridDatum writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMGridDatum(FILE* file);

	//! ngrid_ attribute Getter
	short getNgrid(){return ngrid_;};

	//! ngrid_ attribute Setter
    /*!
      \param ngrid - Integer Number of the predefined grid.
    */
	void setNgrid(short ngrid){ngrid_ = ngrid;};

	//! ndatum_ attribute Getter
	short getNdatum(){return ndatum_;};

	//! ndatum_ attribute Setter
    /*!
      \param ndatum - Integer Number of Predefined Datum.
    */
	void setNdatum(short ndatum){ndatum_ = ndatum;};

	//! dx_ attribute Getter
	short getDx(){return dx_;};

	//! dx_ attribute Setter
    /*!
      \param dz.
    */
	void setDx(short dx){dx_ = dx;};

	//! dy_ attribute Getter
	short getDy(){return dy_;};

	//! dy_ attribute Setter
    /*!
      \param dy.
    */
	void setDy(short dy){dy_ = dy;};

	//! dz_ attribute Getter
	short getDz(){return dz_;};

	//! dz_ attribute Setter
    /*!
      \param dz.
    */
	void setDz(short dz){dz_ = dz;};

	//! origin_ attribute Getter
	double getOrigin(){return origin_;};

	//! origin_ attribute Setter
    /*!
      \param origin.
    */
	void setOrigin(double origin){origin_ = origin;};

	//! falseeast_ attribute Getter
	double getFalseeast(){return falseeast_;};

	//! falseeast_ attribute Setter
    /*!
      \param falseeast.
    */
	void setFalseeast(double falseeast){falseeast_ = falseeast;};

	//! scale1_ attribute Getter
	double getScale1(){return scale1_;};

	//! scale1_ attribute Setter
    /*!
      \param scale1.
    */
	void setScale1(double scale1){scale1_ = scale1;};

	//! falsenorthing_ attribute Getter
	double getFalsenorthing(){return falsenorthing_;};

	//! falsenorthing_ attribute Setter
    /*!
      \param falsenorthing.
    */
	void setFalsenorthing(double falsenorthing){falsenorthing_ = falsenorthing;};

	//! axis_ attribute Getter
	double getAxis(){return axis_;};

	//! axis_ attribute Setter
    /*!
      \param axis.
    */
	void setAxis(double axis){axis_ = axis;};

	//! flattening_ attribute Getter
	double getFlattening(){return flattening_;};

	//! flattening_ attribute Setter
    /*!
      \param flattening.
    */
	void setFlattening(double flattening){flattening_ = flattening;};
};

//! TeGTMInformationImages class with images informations.
/*!
	Attributes used
	name_map - string Image name
	comments - string Image comments
	Gposx - single upper-left corner in degrees
	Gposy - single upper-left corner in degrees
	Gwidth - single Image width in degrees
	Gheight - single Image Height in degrees
	imagelen - long File image length
	Metax - single Reservation – Set to 0
	Metay - single Reservation – Set to 0
	metamapa - byte Reservation – Set to 0
	Tnum - byte Reservation – Set to 1
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMInformationImages
{
private:
	char *name_map_, *comments_, *blob_;//blob -> usado para guardar a imagem em si
	float gposx_, gposy_, gwidth_, gheight_, metax_, metay_;
	long imagelen_;
	unsigned char metamapa_, tnum_;
public:
	//! Constructor
	TeGTMInformationImages();
	
	//! Destructor
	~TeGTMInformationImages();

	//! Images information reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMInformationImages(FILE* file);

	//! Images information writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMInformationImages(FILE* file);

	//! name_map_ attribute Getter
	char* getName_map(){return name_map_;};

	//! name_map_ attribute Setter
    /*!
      \param name_map.
    */
	void setName_map(char* name_map){name_map_ = name_map;};

	//! comments_ attribute Getter
	char* getComments(){return comments_;};

	//! comments_ attribute Setter
    /*!
      \param comments.
    */
	void setComments(char* comments){comments_ = comments;};

	//! gposx_ attribute Getter
	float getGposx(){return gposx_;};

	//! gposx_ attribute Setter
    /*!
      \param gposx.
    */
	void setGposx(float gposx){gposx_ = gposx;};

	//! gposy_ attribute Getter
	float getGposy(){return gposy_;};

	//! gposy_ attribute Setter
    /*!
      \param gposy.
    */
	void setGposy(float gposy){gposy_ = gposy;};

	//! gwidth_ attribute Getter
	float getGwidth(){return gwidth_;};

	//! gwidth_ attribute Setter
    /*!
      \param gwidth.
    */
	void setGwidth(float gwidth){gwidth_ = gwidth;};

	//! gheight_ attribute Getter
	float getGheight(){return gheight_;};

	//! gheight_ attribute Setter
    /*!
      \param gheight.
    */
	void setGheight(float gheight){gheight_ = gheight;};

	//! metax_ attribute Getter
	float getMetax(){return metax_;};

	//! metax_ attribute Setter
    /*!
      \param metax.
    */
	void setMetax(float metax){metax_ = metax;};

	//! metay_ attribute Getter
	float getMetay(){return metay_;};

	//! metay_ attribute Setter
    /*!
      \param metay.
    */
	void setMetay(float metay){metay_ = metay;};

	//! imagelen_ attribute Getter
	long getImagelen(){return imagelen_;};

	//! imagelen_ attribute Setter
    /*!
      \param imagelen.
    */
	void setImagelen(long imagelen){imagelen_ = imagelen;};

	//! metamapa_ attribute Getter
	unsigned char getMetamap(){return metamapa_;};

	//! metamapa_ attribute Setter
    /*!
      \param metamapa.
    */
	void setMetamap(unsigned char metamapa){metamapa_ = metamapa;};

	//! tnum_ attribute Getter
	unsigned char getTnum(){return tnum_;};

	//! tnum_ attribute Setter
    /*!
      \param tnum.
    */
	void setTnum(unsigned char tnum){tnum_ = tnum;};

	//! blob_ attribute Setter
	void setBlob(char* blob){blob_ = blob;};

	//! blob_ attribute Getter (used to store de image in memory)
	char* getBlob(){return blob_;};
};

//! TeGTMWaypoints class with images informations.
/*!
	Attributes used
	latitude - double Latitude in degrees with 13 decimal places
	longitude - double Longitude in degrees with 13 decimal places
	name - String*10 Waypoint Name
	wname - string Waypoint comments
	ico - integer Icon number. See the table below. (Default = 48)
	dslp - byte Display number. See the table below.
	wdate - long Waypoint date - Number of seconds since 31-Dec-1989. See the examples below.
	wrot - integer Angle of Rotation x10 – Example: 1800 = 180 degrees
	walt - single Altitude in meters
	wlayer - integer Reserved – Set to 0
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMWaypoints
{
private:
	double latitude_, longitude_;
	char *name_, *wname_;//name deve ser char[10]
	short ico_, wrot_, wlayer_;
	unsigned char dspl_;
	long wdate_;
	float walt_;
public:
	//! Constructor
	TeGTMWaypoints();

	//! Destructor
	~TeGTMWaypoints();

	//! Images information reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMWaypoints(FILE* file);

	//! Images information writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMWaypoints(FILE* file);

	//! latitude_ attribute Getter
	double getLatitude(){return latitude_;};

	//! latitude_ attribute Setter
    /*!
      \param latitude.
    */
	void setLatitude(double latitude){latitude_ = latitude;};

	//! longitude_ attribute Getter
	double getLongitude(){return longitude_;};

	//! longitude_ attribute Setter
    /*!
      \param longitude.
    */
	void setLongitude(double longitude){longitude_ = longitude;};

	//! name_ attribute Getter
	char* getName(){return name_;};

	//! name_ attribute Setter
    /*!
      \param name.
    */
	void setName(char* name){name_ = name;};

	//! wname_ attribute Getter
	char* getWname(){return wname_;};

	//! wname_ attribute Setter
    /*!
      \param wname.
    */
	void setWname(char* wname){wname_ = wname;};

	//! ico_ attribute Getter
	short getIco(){return ico_;};

	//! ico_ attribute Setter
    /*!
      \param ico.
    */
	void setIco(short ico){ico_ = ico;};

	//! wrot_ attribute Getter
	short getWrot(){return wrot_;};

	//! wrot_ attribute Setter
    /*!
      \param wrot.
    */
	void setWrot(short wrot){wrot_ = wrot;};

	//! wlayer_ attribute Getter
	short getWlayer(){return wlayer_;};

	//! wlayer_ attribute Setter
    /*!
      \param wlayer.
    */
	void setWlayer(short wlayer){wlayer_ = wlayer;};

	//! wdate_ attribute Getter
	long getWdate(){return wdate_;};

	//! wdate_ attribute Setter
    /*!
      \param wdate.
    */
	void setWdate(long wdate){wdate_ = wdate;};

	//! walt_ attribute Getter
	float getWalt(){return walt_;};

	//! walt_ attribute Setter
    /*!
      \param walt.
    */
	void setWalt(float walt){walt_ = walt;};

	//! dspl_ attribute Getter
	unsigned char getDspl(){return dspl_;};

	//! dspl_ attribute Setter
    /*!
      \param dspl.
    */
	void setDspl(unsigned char dspl){dspl_ = dspl;};
};

//! TeGTMWaypoints class with images informations.
/*!
	Attributes used
	height - Long Font Size x –1.333333. Example: If font size is 8, height will be -11
	facename - String Font name – Default = “Times New Roman”
	dspl - Byte Display mode. See the table below.
	color - Long Text color (RGB)
	weight - Long Normal = 400 / Bold = 700
	scale1 - Single Maximum scale in Kilometers that the Waypoint appears on screen (Default = 0)
	border - Byte Border of the background box. See the table below.
	background - Boolean Background box – Used by Text Box Style
	backcolor - Long Color of the Background Box (RGB)
	italic - Byte Normal = 0 / Italic = 1
	underline - Byte Normal = 0 / Underline = 1
	strikeout - Byte Normal = 0 / Strikeout = 1
	alignment - Byte Text alignment in Text Box. See the table below.
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMWaypointsStyles
{
private:
	long height_, color_, weight_, backcolor_;
	char *facename_;
	unsigned char dspl_, border_, italic_, underline_, strikeout_, alignment_;
	float scale1_;
	bool background_;
public:
	//! Constructor
	TeGTMWaypointsStyles();

	//! Destructor
	~TeGTMWaypointsStyles();

	//! Waypoints Styles reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMWaypointsStyles(FILE* file);

	//! Waypoints Styles writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMWaypointsStyles(FILE* file);

	//! height_ attribute Getter
	long getHeight(){return height_;};

	//! height_ attribute Setter
    /*!
      \param height.
    */
	void setHeight(long height){height_ = height;};

	//! color_ attribute Getter
	long getColor(){return color_;};

	//! color_ attribute Setter
    /*!
      \param color.
    */
	void setColor(long color){color_ = color;};

	//! weight_ attribute Getter
	long getWeight(){return weight_;};

	//! weight_ attribute Setter
    /*!
      \param weight.
    */
	void setWeight(long weight){weight_ = weight;};

	//! backcolor_ attribute Getter
	long getBackcolor(){return backcolor_;};

	//! backcolor_ attribute Setter
    /*!
      \param backcolor.
    */
	void setBackcolor(long backcolor){backcolor_ = backcolor;};

	//! facename_ attribute Getter
	char* getFacename(){return facename_;};

	//! facename_ attribute Setter
    /*!
      \param facename.
    */
	void setFacename(char* facename){facename_ = facename;};

	//! dspl_ attribute Getter
	unsigned char getDspl(){return dspl_;};

	//! dspl_ attribute Setter
    /*!
      \param dspl.
    */
	void setDspl(unsigned char dspl){dspl_ = dspl;};

	//! border_ attribute Getter
	unsigned char getBorder(){return border_;};

	//! border_ attribute Setter
    /*!
      \param border.
    */
	void setBorder(unsigned char border){border_ = border;};

	//! italic_ attribute Getter
	unsigned char getItalic(){return italic_;};

	//! italic_ attribute Setter
    /*!
      \param italic.
    */
	void setItalic(unsigned char italic){italic_ = italic;};

	//! underline_ attribute Getter
	unsigned char getunderline(){return underline_;};

	//! underline_ attribute Setter
    /*!
      \param underline.
    */
	void setunderline(unsigned char underline){underline_ = underline;};

	//! strikeout_ attribute Getter
	unsigned char getStrikeout(){return strikeout_;};

	//! strikeout_ attribute Setter
    /*!
      \param strikeout.
    */
	void setStrikeout(unsigned char strikeout){strikeout_ = strikeout;};

	//! alignment_ attribute Getter
	unsigned char getAlignment(){return alignment_;};

	//! alignment_ attribute Setter
    /*!
      \param alignment.
    */
	void setAlignment(unsigned char alignment){alignment_ = alignment;};

	//! scale1_ attribute Getter
	float getScale1(){return scale1_;};

	//! scale1_ attribute Setter
    /*!
      \param scale1.
    */
	void setScale1(float scale1){scale1_ = scale1;};
	
	//! background_ attribute Getter
	bool getBackground(){return background_;};

	//! background_ attribute Setter
    /*!
      \param background.
    */
	void setBackground(bool background){background_ = background;};
};

//! TeGTMTrackLogs class with images informations.
/*!
	Attributes used
	latitude - Double Latitude in degrees with 13 decimal places
	longitude - Double Longitude in degrees with 13 decimal places
	tdate - Long Trackpoint date - Number of seconds since 31-Dec-1989. See the example below.
	tnum - Byte Tracklog flag : 1 = Start a new Tracklog / 0 = continue the tracklog
	talt - Single Altitude in meters
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMTrackLogs
{
private:
	double latitude_, longitude_;
	long tdate_;
	unsigned char tnum_;
	float talt_;
public:
	//! Constructor
	TeGTMTrackLogs();

	//! Destructor
	~TeGTMTrackLogs();

	//! TrackLogs reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMTrackLogs(FILE* file);

	//! TrackLogs writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void writeGTMTrackLogs(FILE* file);

	//! latitude_ attribute Getter
	double getLatitude(){return latitude_;};

	//! latitude_ attribute Setter
    /*!
      \param latitude.
    */
	void setLatitude(double latitude){latitude_ = latitude;};

	//! longitude_ attribute Getter
	double getLongitude(){return longitude_;};

	//! longitude_ attribute Setter
    /*!
      \param longitude.
    */
	void setLongitude(double longitude){longitude_ = longitude;};

	//! tdate_ attribute Getter
	long getTdate(){return tdate_;};

	//! tdate_ attribute Setter
    /*!
      \param tdate.
    */
	void setTdate(long tdate){tdate_ = tdate;};

	//! tnum_ attribute Getter
	unsigned char getTnum(){return tnum_;};

	//! tnum_ attribute Setter
	/*!
      \param tnum.
    */
	void setTnum(unsigned char tnum){tnum_ = tnum;};

	//! talt_ attribute Getter
	float getTalt(){return talt_;};

	//! talt_ attribute Setter
    /*!
      \param talt.
    */
	void setTalt(float talt){talt_ = talt;};
};

//! TeGTMTrackLogs class with images informations.
/*!
	Attributes used
	tname - String Tracklog name
	ttype - Byte Tracklog type. See the table below.
	tcolor - Long Tracklog color (RGB)
	tscale - Single Maximum scale in Kilometers that the Tracklog appears on screen (Default = 0)
	tlabel - Byte Reservation – Set to 0
	tlayer - Integer Reservation – Set to 0
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMTrackLogStyles
{
private:
	char* tname_;
	long tcolor_;
	unsigned char ttype_, tlabel_;
	float tscale_;
	short tlayer_;
public:
	//! Constructor
	TeGTMTrackLogStyles();

	//! Destructor
	~TeGTMTrackLogStyles();

	//! TrackLogs Styles reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMTrackLogStyles(FILE* file);

	//! TrackLogs Styles writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMTrackLogStyles(FILE* file);

	//! tname_ attribute Getter
	char* getTname(){return tname_;};

	//! tname_ attribute Setter
    /*!
      \param tname.
    */
	void setTname(char* tname){tname_ = tname;};

	//! tcolor_ attribute Getter
	long getTcolor(){return tcolor_;};

	//! tcolor_ attribute Setter
    /*!
      \param tcolor.
    */
	void setTcolor(long tcolor){tcolor_ = tcolor;};

	//! ttype_ attribute Getter
	unsigned char getTtype(){return ttype_;};

	//! ttype_ attribute Setter
    /*!
      \param ttype.
    */
	void setTtype(unsigned char ttype){ttype_ = ttype;};

	//! tlabel_ attribute Getter
	unsigned char getTlabel(){return tlabel_;};

	//! tlabel_ attribute Setter
    /*!
      \param tlabel.
    */
	void setTlabel(unsigned char tlabel){tlabel_ = tlabel;};

	//! tscale_ attribute Getter
	float getTscale(){return tscale_;};

	//! tscale_ attribute Setter
    /*!
      \param tscale.
    */
	void setTscale(float tscale){tscale_ = tscale;};

	//! tlayer_ attribute Getter
	short getTlayer(){return tlayer_;};

	//! tlayer_ attribute Setter
    /*!
      \param tlayer.
    */
	void setTlayer(short tlayer){tlayer_ = tlayer;};
};

//! TeGTMRoutes class with images informations.
/*!
	Attributes used
	latitude - Double Latitude in degrees with 13 decimal places
	longitude - Double Longitude in degrees with 13 decimal places
	wname - String *10 Routepoint Name
	wcomment - String Routepoint Comments
	rname - String Route name
	ico - Integer Icon number . See the table of Icons. (Default = 48)
	dspl - Byte Display number. See the table of dspl.
	tnum - Byte Route flag : 1 = Start a new Route / 0 = continue the route
	tdate - Long Reservation – Set to 0
	wrot - Integer Reservation – Set to 0
	walt - Single Altitude in meters
	wlayer - Integer Reservation – Set to 0
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMRoutes
{
private:
	double latitude_, longitude_;
	char* wname_, *wcomment_, *rname_;//wname deve ser char[10]
	short ico_, wrot_, wlayer_;
	unsigned char dspl_, tnum_;
	long tdate_;
	float walt_;
public:
	//! Constructor
	TeGTMRoutes();

	//! Destructor
	~TeGTMRoutes();

	//! TrackLogs Styles reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMRoutes(FILE* file);

	//! TrackLogs Styles writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMRoutes(FILE* file);

	//! latitude_ attribute Getter
	double getLatitude(){return latitude_;};

	//! latitude_ attribute Setter
    /*!
      \param latitude.
    */
	void setLatitude(double latitude){latitude_ = latitude;};

	//! longitude_ attribute Getter
	double getLongitude(){return longitude_;};

	//! longitude_ attribute Setter
    /*!
      \param longitude.
    */
	void setLongitude(double longitude){longitude_ = longitude;};

	//! wname_ attribute Getter
	char* getWname(){return wname_;};

	//! wname_ attribute Setter
    /*!
      \param wname.
    */
	void setWname(char* wname){wname_ = wname;};

	//! wcomment_ attribute Getter
	char* getWcomment(){return wcomment_;};

	//! wcomment_ attribute Setter
    /*!
      \param wcomment.
    */
	void setWcomment(char* wcomment){wcomment_ = wcomment;};

	//! rname_ attribute Getter
	char* getRname(){return rname_;};

	//! rname_ attribute Setter
    /*!
      \param rname.
    */
	void setRname(char* rname){rname_ = rname;};

	//! ico_ attribute Getter
	short getIco(){return ico_;};

	//! ico_ attribute Setter
    /*!
      \param ico.
    */
	void setIco(short ico){ico_ = ico;};

	//! wrot_ attribute Getter
	short getWrot(){return wrot_;};

	//! wrot_ attribute Setter
    /*!
      \param wrot.
    */
	void setWrot(short wrot){wrot_ = wrot;};

	//! wlayer_ attribute Getter
	short getWlayer(){return wlayer_;};

	//! wlayer_ attribute Setter
    /*!
      \param wlayer.
    */
	void setWlayer(short wlayer){wlayer_ = wlayer;};

	//! dspl_ attribute Getter
	unsigned char getDspl(){return dspl_;};

	//! dspl_ attribute Setter
    /*!
      \param dspl.
    */
	void setDspl(unsigned char dspl){dspl_ = dspl;};

	//! tnum_ attribute Getter
	unsigned char getTnum(){return tnum_;};

	//! tnum_ attribute Setter
    /*!
      \param tnum.
    */
	void setTnum(unsigned char tnum){tnum_ = tnum;};

	//! tdate_ attribute Getter
	long getTdate(){return tdate_;};

	//! tdate_ attribute Setter
    /*!
      \param tdate.
    */
	void setTdate(long tdate){tdate_ = tdate;};

	//! walt_ attribute Getter
	float getWalt(){return walt_;};

	//! walt_ attribute Setter
    /*!
      \param walt.
    */
	void setWalt(float walt){walt_ = walt;};
};

//! TeGTMLayers class with images informations.
/*!
	Not implemented yet
	Attributes used
	index - Integer Layer index
	name - String Layer name
	color - Long Layer color
	lock - Byte Flag – Layer lock
	view - Byte Flag – layer visible
	reserva1 - Byte Reservation
	reserva2 - Integer Reservation
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMLayers
{
private:
	short index_, reserva2_;
	char* name_;
	long color_;
	unsigned char lock_, view_, reserva1_;
public:
	//! Constructor
	TeGTMLayers();

	//! Destructor
	~TeGTMLayers();

	//! Layers reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMLayers(FILE* file);
	//! Layers writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writerGTMLayers(FILE* file);

	//! index_ attribute Getter
	short getIndex(){return index_;};
	//! index_ attribute Setter
    /*!
      \param index.
    */
	void setIndex(short index){index_ = index;};

	//! reserva2_ attribute Getter
	short getReserva2(){return reserva2_;};
	//! reserva2_ attribute Setter
    /*!
      \param reserva2.
    */
	void setReserva2(short reserva2){reserva2_ = reserva2;};

	//! name_ attribute Getter
	char* getName(){return name_;};
	//! name_ attribute Setter
    /*!
      \param name.
    */
	void setName(char* name){name_ = name;};

	//! color_ attribute Getter
	long getColor(){return color_;};
	//! color_ attribute Setter
    /*!
      \param color.
    */
	void setColor(long color){color_ = color;};

	//! lock_ attribute Getter
	unsigned char getLock(){return lock_;};
	//! lock_ attribute Setter
    /*!
      \param lock.
    */
	void setLock(unsigned char lock){lock_ = lock;};
	
	//! view_ attribute Getter
	unsigned char getView(){return view_;};
	//! view_ attribute Setter
    /*!
      \param view.
    */
	void setView(unsigned char view){view_ = view;};

	//! reserva1_ attribute Getter
	unsigned char getReserva1(){return reserva1_;};
	//! reserva1_ attribute Setter
    /*!
      \param reserva1.
    */
	void setReserva1(unsigned char reserva1){reserva1_ = reserva1;};
};

//! TeGTMUserIcons class with images informations.
/*!
	Not implemented yet
	Attributes used
	name - Double Name of user icor
	number - Double Number of user icon
	size - String *10 Size of the image of icon
	\author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTMUserIcons
{
private:
	char* name_;
	unsigned char number_;
	long size_;
public:
	//! Constructor
	TeGTMUserIcons();

	//! Destructor
	~TeGTMUserIcons();

	//! User icons reader
    /*!
      \param file pointer to a FILE* with the .gtm file to be imported.
    */
	void readGTMUserIcons(FILE* file);

	//! User icons writer
    /*!
      \param file pointer to a FILE* with the .gtm file to be exported.
    */
	void writeGTMUserIcons(FILE* file);

	//! name_ attribute Getter
	char* getName(){return name_;};

	//! name_ attribute Setter
    /*!
      \param name.
    */
	void setName(char* name){name_ = name;};

	//! number_ attribute Getter
	unsigned char getNumber(){return number_;};

	//! number_ attribute Setter
    /*!
      \param number.
    */
	void setNumber(unsigned char number){number_ = number;};

	//! size_ attribute Getter
	long getSize(){return size_;};

	//! size_ attribute Setter
    /*!
      \param size.
    */
	void setSize(long size){size_ = size;};
};

//! TeGTM .gtm files driver class.
/*!
	This class contains the implementation of common methods for TerraLib access .gtm files.   
    For further information about the GTM file format access http://www.gpstm.com/download/GTM211_format.pdf
    \author Luiz Claudio Oliveira de Andrade <lcoandrade@gmail.com>    
 */	
class TDKEB_DGNIMPORT_API TeGTM
{
private:
	TeGTMHeader header_;
	TeGTMGridDatum griddatum_;
	vector<TeGTMInformationImages*> informationimages_;
	vector<TeGTMWaypoints*> waypoints_;
	vector<TeGTMWaypointsStyles*> waypointsstyles_;
	vector<TeGTMTrackLogs*> tracklogs_;
	vector<TeGTMTrackLogStyles*> tracklogstyles_;
	vector<TeGTMRoutes*> routes_;
	vector<TeGTMUserIcons*> usericons_;
	vector<TeGTMLayers*> layers_;
	std::string layersNames_;

	std::map<std::string, std::vector<TePoint> > waypointsLayer;
	std::map<std::string, std::vector<TePoint> > tracklogsLayer;
	std::map<std::string, std::vector<TePoint> > routesLayer;
	std::map<std::string, std::vector<char*> > imagesLayer;

	TeTable waypointTable_;
	TeTable tracklogTable_;
	TeTable routeTable_;

	FILE* file_;

	std::vector<char*> icon_descr;

public:
	//! Constructor
	TeGTM(FILE* file);

	//! Destructor
	~TeGTM();

	//! GTM file reader
	void readGTM();

	//! GTM file writer
	void writeGTM();

	//! TeProjection attribute Getter
	TeProjection* getProjection();

	//! TeProjection attribute Setter
    /*!
      \param proj - projection to be set.
    */
	TeProjection* setProjection(TeProjection* proj);

	//! TeDatum attribute Getter
	TeDatum getDatum();

	//! TeDatum attribute Setter
    /*!
      \param datum.
    */
	short setDatum(TeDatum datum);

	//! Layer names Getter
	std::string getLayerNames(std::string fileName);

	//! Returns the date string calculated with the date(Number of seconds since 31-Dec-1989).
	std::string getDate(long date);

	//! GTM importer,  converts de GTM loaded in memory to TeDatabase
    /*!
      \param db - Database used
	  \param fileName - GTM file name
    */
	std::vector<TeLayer*> importGTM(TeDatabase *db, std::string fileName);

	//! GTM exporter,  converts TeTheme into a GTM file
    /*!
      \param theme - TeTheme to be exported
    */
	void exportGTM(TeTheme *theme, std::string fileName, FILE* file);

	//! Creates the attribute list used by the waypoints
    /*!
      \param attList - List of columns attributes
    */
	void waypointAttributeList(TeAttributeList& attList);

	//! Creates the attribute list used by the waypoints
    /*!
      \param attList - List of columns attributes
    */
	void tracklogAttributeList(TeAttributeList& attList);

	//! Creates the attribute list used by the waypoints
    /*!
      \param attList - List of columns attributes
    */
	void routeAttributeList(TeAttributeList& attList);

	//! Waypoint importer
    /*!
      \param layer - Layer created to receive the data
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param chunkSize - Size used to save transactions(set to 100)
    */
	bool importWaypoints(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize );

	//! Tracklog importer (TePOINTS)
    /*!
      \param layer - Layer created to receive the data
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param chunkSize - Size used to save transactions(set to 100)
    */
	bool importTracklogs(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize );

	//! Tracklog importer (TeLINES)
    /*!
      \param layer - Layer created to receive the data
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param chunkSize - Size used to save transactions(set to 100)
    */
	bool importTracklogs2(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize );

	//! Routes importer (TePOINTS)
    /*!
      \param layer - Layer created to receive the data
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param chunkSize - Size used to save transactions(set to 100)
    */
	bool importRoutes(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize );

	//! Routes importer (TeLINES)
    /*!
      \param layer - Layer created to receive the data
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param chunkSize - Size used to save transactions(set to 100)
    */
	bool importRoutes2(TeLayer* layer, string attrTableName,string objectIdAtt,int unsigned chunkSize );

	//! Images importer
    /*!
      \param layer - Layer created to receive the data
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param chunkSize - Size used to save transactions(set to 100)
    */
	int importImages(TeLayer* layer);

	//! Create the attribute table used by the importGTM method
    /*!
      \param layer - Layer created to receive the data
	  \param attList - List of columns attributes
	  \param attrTableName - Name of the attribute table
	  \param objectIdAtt - colum used as link
	  \param oldPrec - layer precision
	  \param linkCol - colum id used as link
    */
	bool createAttributeTable(TeLayer* layer, TeAttributeList attList, std::string attrTableName, std::string objectIdAtt, 
								double oldPrec, int& linkCol);
};

#endif