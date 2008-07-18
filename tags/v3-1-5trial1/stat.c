/*
 * Copyright (C) 1996-2001, Thomas Andrews
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "math.h"
#include "deal.h"
#include "stat.h"
#include "keywords.h"
#include "tcl.h"

double sdev_data(double weight,double sum, double squares) {
  return sqrt((squares-(sum*sum)/weight)/(weight-1));
}

double rms_data(double weight,double sum,double squares) {
  return sqrt((squares-(sum*sum)/weight)/weight);
}

double sdev(sd)
     SDev *sd;
{
  double xx=sd->sumsquared,xy=sd->sum;
  double weight=sd->weight;
  return sdev_data(weight,xy,xx);
}

double sdevAverage(sd)
     SDev *sd;
{
  return sd->sum/sd->weight;
}

void sdevAddData(sd,weight,data) 
     SDev *sd;
     double weight;
     double data;
{
  if (sd->count==0) { 
	sd->max=data; sd->min=data;
  } else {
        if ((sd->max) < data) { (sd->max)=data; }
        if ((sd->min) > data) { (sd->min)=data; }
  }
 
  sd->count++;
  sd->weight += weight;
  sd->sum += (weight*data); 
  sd->sumsquared += (weight*data*data);
}

void sdevFree(ClientData sdev) {
  Tcl_Free((char *)sdev);
}
	
void sdevReset(SDev *sdev) {
  sdev->sum=0.0;
  sdev->sumsquared=0.0;
  sdev->weight=0.0;
  sdev->count=0;
}

SDev *sdevNew() {
  SDev *sdev=(SDev *)Tcl_Alloc(sizeof(SDev));
  sdevReset(sdev);
  return sdev;
}

void corrAddData(corr,weight,x,y)
     Correlation *corr;
	 double weight;
     double x,y;
{
  corr->count++;
  corr->weight += weight;
  corr->sumx += (weight*x);
  corr->sumxx += (weight*x*x);
  corr->sumy += (weight*y);
  corr->sumyy += (weight*y*y);
  corr->sumxy += (weight*x*y);
}

static double deviationsq(double weight,
			  double sumu,
			  double sumv,
			  double sumuv)
{
  return (sumuv*weight-sumu*sumv);
}

double corrResult(Correlation *corr)
{
  double x,y,xy;
  if (corr->count==0) { return -2; }
  x=deviationsq(corr->weight,corr->sumx,corr->sumx,corr->sumxx);
  y=deviationsq(corr->weight,corr->sumy,corr->sumy,corr->sumyy);
  xy=deviationsq(corr->weight,corr->sumx,corr->sumy,corr->sumxy);
  return (xy/sqrt(x*y));
}

Correlation *correlationNew() {
  Correlation *corr=(Correlation *)Tcl_Alloc(sizeof(Correlation));
  corrReset(corr);
  return corr;
}

void corrReset(Correlation *corr) {
  corr->count=0;
  corr->weight=0.0;
  corr->sumx=0.0; 
  corr->sumxx=0.0;
  corr->sumy=0.0;
  corr->sumyy=0.0;
  corr->sumxy=0.0;
}

void correlationFree(ClientData corr) {
  Tcl_Free((char *)corr);
}

int count(sd)
SDev *sd;
{
	return sd->count;
}


int tcl_sdev_command ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  SDev *sd=(SDev *)cd;
  static int
    addCommandID=-1,
	addwCommandID=-1,
    averageCommandID=-1,
    deviationCommandID=-1,
    countCommandID=-1,
	weightCommandID=-1,
    rmsCommandID=-1,
    minCommandID=-1,
    maxCommandID=-1,
    initialize=1;

  int cmd;

  if (initialize) {
      initialize=0;
      addCommandID=Keyword_addKey("add");
	  addwCommandID=Keyword_addKey("addw");
	  weightCommandID=Keyword_addKey("weight");
      averageCommandID=Keyword_addKey("average");
      deviationCommandID=Keyword_addKey("sdev");
      countCommandID=Keyword_addKey("count");
      rmsCommandID=Keyword_addKey("rms");
      minCommandID=Keyword_addKey("min");
      maxCommandID=Keyword_addKey("max");
  }

  cmd=Keyword_getIdFromObj(interp,objv[1]);

  if (cmd==addCommandID && objc>=3) {
    double data;
    int i;
    for (i=2; i<objc; i++) {
      if (TCL_OK!=Tcl_GetDoubleFromObj(interp,objv[i],&data)) {
	    return TCL_ERROR;
      }
      sdevAddData(sd,1.0,data);
    }
    return TCL_OK;
  }

  if (cmd==addwCommandID && objc>=4) {
	  double weight,data;
	  int i;

	  if (TCL_OK!=Tcl_GetDoubleFromObj(interp,objv[2],&weight)) {
		  return TCL_ERROR;
      }

      if (weight<0.0) { return TCL_ERROR; }

      for (i=3; i<objc; i++) {
        if (TCL_OK!=Tcl_GetDoubleFromObj(interp,objv[i],&data)) {
	      return TCL_ERROR;
		}
        sdevAddData(sd,weight,data);
	  }
	  return TCL_OK;
  }

  if (cmd==countCommandID && objc == 2) {
	Tcl_Obj *obj=Tcl_NewIntObj(count(sd));
	Tcl_SetObjResult(interp,obj);
	return TCL_OK;
  }

  if (cmd==maxCommandID && objc == 2) {
	  Tcl_Obj *obj=Tcl_NewDoubleObj(sd->max);
	  Tcl_SetObjResult(interp,obj);
	  return TCL_OK;
  }
  if (cmd==minCommandID && objc == 2) {
	  Tcl_Obj *obj=Tcl_NewDoubleObj(sd->min);
	  Tcl_SetObjResult(interp,obj);
	  return TCL_OK;
  }
  if (cmd==weightCommandID && objc == 2) {
	  Tcl_Obj *obj=Tcl_NewDoubleObj(sd->weight);
	  Tcl_SetObjResult(interp,obj);
	  return TCL_OK;
  }

  if (sd->count==0 || sd->weight==0.0) {
    Tcl_AddErrorInfo(interp,"Can not do statistical computation without data");
    return TCL_ERROR;
  }

  if (cmd==rmsCommandID && objc==2) {
    Tcl_Obj *obj=Tcl_NewDoubleObj(rms_data(sd->weight,sd->sum,sd->sumsquared));
    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }

  if (cmd==averageCommandID && objc==2) {
    double result=sdevAverage(sd);
    Tcl_Obj *obj=Tcl_NewDoubleObj(result);
    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }
  
  if (sd->count==1) {
    Tcl_AddErrorInfo(interp,"Cannot compute deviation on one point of data");
    return TCL_ERROR;
  }

  if (cmd==deviationCommandID && objc==2) {
    double result=sdev(sd);
    Tcl_Obj *obj=Tcl_NewDoubleObj(result);
    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }

  return TCL_ERROR;
}

int tcl_correlate_command (TCLOBJ_PARAMS) TCLOBJ_DECL
{
  static int addCommandID=-1,
	  addwCommandID=-1,
    correlateCommandID=-1,
    averageCommandID=-1,
	weightCommandID=-1,
    sdevCommandID=-1,
    rmsCommandID=-1,
    xID=-1,
    yID=-1,
    countCommandID=-1,
    initialize=1;
  int command;
  Correlation *corr=(Correlation *)cd;

  if (initialize) {
    initialize=0;
    addCommandID=Keyword_addKey("add");
    addwCommandID=Keyword_addKey("addw");
    correlateCommandID=Keyword_addKey("correlate");
    averageCommandID=Keyword_addKey("average");
    sdevCommandID=Keyword_addKey("sdev");
    countCommandID=Keyword_addKey("count");
    weightCommandID=Keyword_addKey("weight");
    rmsCommandID=Keyword_addKey("rms");
    correlateCommandID=Keyword_addKey("correlate");
    xID=Keyword_addKey("x");
    yID=Keyword_addKey("y");
  }

  if (objc==1) { goto corrusage; }

  command=Keyword_getIdFromObj(interp,objv[1]);

  if (KEYWORD_INVALID_ID==command) {
    goto corrusage;
  }

  if (command==addCommandID && objc>=3) {
    double value1;
    double value2;
    int i;

    if (objc%2==1) {
      Tcl_AddErrorInfo(interp,"Data must be added in pairs - got an odd number of datapoints");
      return TCL_ERROR;
    }

    for (i=2; i<objc; i+=2) {
      int result=Tcl_GetDoubleFromObj(interp,objv[i],&value1);
      if (result!=TCL_OK) { goto corrusage; }
      result=Tcl_GetDoubleFromObj(interp,objv[i+1],&value2);
      if (result!=TCL_OK) { goto corrusage; }
      corrAddData(corr,1.0,value1,value2);
    }
    return TCL_OK;
  }

  if (command==addwCommandID && objc>=4) {
	double weight;
    double value1;
    double value2;
    int i;

	if (TCL_OK!=Tcl_GetDoubleFromObj(interp,objv[2],&weight)) {
	  return TCL_ERROR;
    }

    if (weight<0.0) { return TCL_ERROR; }


    if (objc%2==0) {
      Tcl_AddErrorInfo(interp,"Data must be added in pairs - got an odd number of datapoints");
      return TCL_ERROR;
    }

    for (i=3; i<objc; i+=2) {
      int result=Tcl_GetDoubleFromObj(interp,objv[i],&value1);
      if (result!=TCL_OK) { goto corrusage; }
      result=Tcl_GetDoubleFromObj(interp,objv[i+1],&value2);
      if (result!=TCL_OK) { goto corrusage; }
      corrAddData(corr,weight,value1,value2);
    }
    return TCL_OK;
  }


  if (command==countCommandID && objc==2) {
    Tcl_Obj *obj=Tcl_NewIntObj(corr->count);
    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }

  if (command==weightCommandID && objc==2) {
    Tcl_Obj *obj=Tcl_NewDoubleObj(corr->weight);
    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }

  if (corr->count==0) {
    Tcl_AddErrorInfo(interp,"Cannot compute values without data");
    return TCL_ERROR;
  }


  if (command==averageCommandID && objc==3) {
    double value;
    int var=Keyword_getIdFromObj(interp,objv[2]);
    if (var==xID) {
      value=corr->sumx/corr->count;
    } else if (var==yID) {
      value=corr->sumy/corr->count;
    } else {
      goto corrusage;
    }
    Tcl_SetObjResult(interp,Tcl_NewDoubleObj(value));
    return TCL_OK;
  }

  if (command==rmsCommandID && objc==3) {
    double value;
    int var=Keyword_getIdFromObj(interp,objv[2]);
    if (var==xID) {
      value=rms_data(corr->count,corr->sumx,corr->sumxx);
    } else if (var==yID) {
      value=rms_data(corr->count,corr->sumy,corr->sumyy);
    } else {
      goto corrusage;
    }
    Tcl_SetObjResult(interp,Tcl_NewDoubleObj(value));
    return TCL_OK;
  }

  if (corr->count<=1) {
    Tcl_AddErrorInfo(interp,"Cannot compute correlation or standard deviation\n");
    Tcl_AddErrorInfo(interp,"Without at least two points of data");
    return TCL_ERROR;
  }

  if (command==correlateCommandID && objc==2) {
    double value=corrResult(corr);
    Tcl_Obj *obj=Tcl_NewDoubleObj(value);
    Tcl_SetObjResult(interp,obj);
    return TCL_OK;
  }

  if (command==sdevCommandID && objc==3) {
    double value;
    int var=Keyword_getIdFromObj(interp,objv[2]);
    if (var==xID) {
      value=sdev_data(corr->count,corr->sumx,corr->sumxx);
    } else if (var==yID) {
      value=sdev_data(corr->count,corr->sumy,corr->sumyy);
    } else {
      goto corrusage;
    }
    Tcl_SetObjResult(interp,Tcl_NewDoubleObj(value));
    return TCL_OK;
  }

 corrusage:
  return TCL_ERROR;
}

int tcl_sdev_define ( TCL_PARAMS ) TCL_DECL
{
  CONST84 char *name=argv[1];
  SDev *sd;
  argc--; argv++;
  sd=sdevNew();
  Tcl_CreateObjCommand(interp,name,tcl_sdev_command,(ClientData)sd,sdevFree);
  return TCL_OK;
}

int tcl_correlation_define ( TCLOBJ_PARAMS ) TCLOBJ_DECL
{
  char *name=Tcl_GetString(objv[1]);
  Correlation *corr;
  corr=correlationNew();
  Tcl_CreateObjCommand(interp,name,tcl_correlate_command,(ClientData)corr,correlationFree);
  return TCL_OK;
}
