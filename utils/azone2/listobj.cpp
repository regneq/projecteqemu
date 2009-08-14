/*

  List Placeable Objects in an S3D or EQG. 
  By Derision, based on OpenEQ File Loaders by Daeken et al.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY except by those people which sell it, which
  are required to give you total support for your newly bought product;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef          short SHORT;
typedef unsigned long  DWORD;

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "wld.hpp"

#include "archive.hpp"
#include "pfs.hpp"

#include "file_loader.hpp"
#include "zon.hpp"
#include "ter.hpp"



bool ProcessZoneFile(const char *shortname);
void ListPlaceable(FileLoader *fileloader, char *ZoneFileName);
enum EQFileType { S3D, EQG, UNKNOWN };


int main(int argc, char *argv[]) {

	printf("LISTOBJ: List Placeable Objects in .S3D or .EQG zone files.\n");
	
	if(argc != 2) {
		printf("Usage: %s (zone short name)\n", argv[0]);
		return(1);
	}
	
	return(ProcessZoneFile(argv[1]));
}

bool ProcessZoneFile(const char *shortname) {
	
	char bufs[96];
  	Archive *archive;
  	FileLoader *fileloader;
  	Zone_Model *zm;
	FILE *fff;
	EQFileType FileType = UNKNOWN;
	
	sprintf(bufs, "%s.s3d", shortname);

	archive = new PFSLoader();
	fff = fopen(bufs, "rb");
	if(fff != NULL) 
		FileType = S3D;
	else {
		sprintf(bufs, "%s.eqg", shortname);
		fff = fopen(bufs, "rb");
		if(fff != NULL)
			FileType = EQG;
	}

	if(FileType == UNKNOWN) {
		printf("Unable to locate %s.s3d or %s.eqg\n", shortname, shortname);
		return(false);
	}

  	if(archive->Open(fff) == 0) {
		printf("Unable to open container file '%s'\n", bufs);
		return(false);
	}

	switch(FileType) {
		case S3D: 
  			fileloader = new WLDLoader();
  			if(fileloader->Open(NULL, (char *) shortname, archive) == 0) {
	  			printf("Error reading WLD from %s\n", bufs);
	  			return(false);
  			}
			break;
		case EQG:
			fileloader = new ZonLoader();
			if(fileloader->Open(NULL, (char *) shortname, archive) == 0) {
				printf("Error reading ZON/TER from %s\n", bufs);
				return(false);
	        	}
			break;
		case UNKNOWN:
			break;
	}


	zm = fileloader->model_data.zone_model;
  
	ListPlaceable(fileloader, bufs);

	return(true);
}

void ListPlaceable(FileLoader *fileloader, char *ZoneFileName) {

	for(int i = 0; i < fileloader->model_data.plac_count; ++i) {
		if(fileloader->model_data.placeable[i]->model==-1) continue;
		if(fileloader->model_data.models[fileloader->model_data.placeable[i]->model] == NULL) continue;
		printf("Placeable Object %4d @ (%9.2f, %9.2f, %9.2f uses model %4d %s\n",i,
	       	 	fileloader->model_data.placeable[i]->y,
	        	fileloader->model_data.placeable[i]->x,
	        	fileloader->model_data.placeable[i]->z,
			fileloader->model_data.placeable[i]->model,
			fileloader->model_data.models[fileloader->model_data.placeable[i]->model]->name); 

		//if((fileloader->model_data.placeable[i]->scale[0] != 1) ||
		//  (fileloader->model_data.placeable[i]->scale[1] != 1))
		//if((fileloader->model_data.placeable[i]->scale[0] != (fileloader->model_data.placeable[i]->scale[1])))
		//if((fileloader->model_data.placeable[i]->scale[0] != 1.0))
		//printf("   XScale = %9.2f, YScale = %9.2f, ZScale = %9.2f\n",
		//	fileloader->model_data.placeable[i]->scale[0],
		//	fileloader->model_data.placeable[i]->scale[1],
		//	fileloader->model_data.placeable[i]->scale[2]);


	}
}

			
