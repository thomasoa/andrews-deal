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

#include "tcl_incl.h"
typedef struct {
	int count;
	double weight;
	double sum,sumsquared;
        double max, min;
} SDev;

typedef struct {
	int count;
	double weight;
        double maxx, maxy;
        double minx, miny;
	double sumx,sumy,sumxx,sumyy,sumxy;
} Correlation;

double sdev PROTO((SDev *));
double sdevAverage PROTO((SDev *));
void sdevReset PROTO((SDev *));
void sdevAddData PROTO((SDev *,double,double));
SDev *sdevNew PROTO(());
void sdevFree PROTO((ClientData));
int tcl_rand_cmd (TCLOBJ_PARAMS);
int tcl_sdev_define(TCL_PARAMS);
int tcl_correlation_define(TCLOBJ_PARAMS);


Correlation *correlationNew PROTO(());
void corrAddData PROTO((Correlation *,double,double,double));
double corrResult PROTO((Correlation *corr));
void corrReset PROTO((Correlation *));
void correlationFree PROTO((ClientData));
