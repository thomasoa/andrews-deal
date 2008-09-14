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


typedef struct lazy_vector_data {
  int num;
  int value[13];
} *LazyVectorData;

LazyVectorData newLazyVector PROTO( () );

typedef int *VectorTable;

#define VectorTableLookup(table,holding) \
	table[1+((8191&holding)>>(table[0]))]

int vectorCount PROTO( (VectorTable,int,int) );


/*
 *
 * Gross - should define this as a static function and
 * hope the compiler inlines the code.
 *
 */
#define SetVectorData(vector,i,val) \
            {\
	      int ___i=i,___val=(val); LazyVectorData ___vec=(vector);\
		if (___i>=0 && ___i<=13) {\
		  ___vec->value[___i]=(___val);\
		    if (___i>___vec->num && (___val!=0)) { ___vec->num=___i; }\
		}\
            }
