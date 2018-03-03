/*
 * ScenGene.cc
 *
 *  Created on: Feb 8, 2018
 *      Author: js
 */
#include "ScenGene.h"
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <float.h>

using namespace std;


class ScenGene;

const static char digits[11] = {'0','1','2','3','4','5','6','7','8','9','\0'};
const static string key_Input[8] = {"____","**","__node__","__edge__","__route__","__file__","__wired__"};
const static string key_node[12] = { "node_prefix","rowc","colc","xmax","ymax","axis_type","radius","xstart","ystart","base_start","junc_type","junc_width"};
const static string key_edge[4] = {"lanes","bidirection","connectivity","edge_prefix"};
const static string key_route[22] = {
					"vtype_prefix","vtype_num", "vtype_accel", "vtype_decel",   "vtype_length", "vtype_Maxspeed","vtype_width","vtype_sigma",
					"trip_prefix", "trip_index","trip_num",    "edge_rangeBase","edge_rangeMax","edge_start",
					"vehi_prefix", "vehi_num",  "vehi_types",  "vehi_routes",   "vehi_depart",  "vehi_color",
					"poisson_lambda","trip_divide_num"};
const static string key_file[7] = {"name","node_xml","edge_xml","route_xml","base_tcl","wired_tcl","connect_tcl"};
const static string key_wired[3] = {"junc_Only","xdist_base","ydist_base"};
const static string nodeTypeStr[11]={"priority",
								"traffic_light",
								"right_before_left",
								"unregulated",
								"traffic_light_unregulated",
								"priority_stop",
								"allway_stop",
								"rail_signal",
								"zipper",
								"rail_crossing",
								"traffic_light_right_on_red"};




int main(void){
	ScenGene *sg = new ScenGene();
	sg->readconf();
	sg->file_nodxml();
	sg->file_edgxml();
	sg->file_rouxml();
	sg->file_bastcl();
	sg->file_wirtcl();
	sg->file_pattcl();
}


ScenGene::ScenGene(){

//simulator--------------------scen & ..
	scen_xlength = 0.0;
	scen_ylength = 0.0;
	simu_prefix = "ns_";
	file_prefix = "Vanet";
//node---------------junctions &  base stations
	node_prefix = "";
	node_rowc = 0;
	node_colc = 0;
	node_radius = 0.0;
	node_xstart = 0.0;
	node_ystart = 0.0;
	node_axis_type_bits = 0;
	node_junc_type = 0;
	node_base_station_start = 0;
	node_junc_width = 0;
//edge----------------------streets & lanes &  links
	edge_prefix = "";
	edge_lane_num = 0;
	edge_bidirection_bits = 0;
	edge_connect_bits = 0;
//route-----------------types of vehicular & trips & vehicle path
	route_vtypes_num = 0;
	route_vtypes_accel[0] = 0.0;
	route_vtypes_decel[0] = 0.0;
	route_vtypes_length[0] = 0;
	route_vtypes_width[0] = 0;
	route_vtypes_maxspeed = 0.0;
	route_vtypes_sigma[0] = 0.0;

	route_trip_prefix = "";
	route_trip_start = 0;
	route_trip_num = 0;
	route_trip_divide_num = 0;

	route_edge_range_base = 0;
	route_edge_range_max = 0;
	route_edge_start = "";

	route_vehi_prefix = 0;
	route_vehi_num = 0;
	route_vehi_types = 0; // derived from route_vtypes_num
	route_vehi_routes = 0; // derived from route_trip_num
	route_vehi_depart = 0; // poisson distribution
	route_vehi_color_bits = 0;

//Poisson distribution--------------------lambda
	lambda = 0;

//I/O-----------------files specification
	confFile = "scenParameter.conf";

	nodeFile = file_prefix + ".nod.xml";
	edgeFile = file_prefix + ".edg.xml";
	routeFile = file_prefix + ".rou.xml";
	settingFile = file_prefix + ".settings.xml";
	cfgFile = file_prefix + ".sumocfg";

	base_xstep = 0.0;
	base_ystep = 0.0;

	base_loc = new double*[2];
	for(int i = 0; i < 2; i++){
		base_loc[i] = new double[1];
	}

	base_startIndex = 0;

	wired_junc_only = -1;

	vehibases_num = 0;

	wired_xdist_base = 0;
	wired_ydist_base = 0;

}

ScenGene::~ScenGene(){
	delete base_loc;
}


void
ScenGene::parseLine(int type, int index){
	if(current_line == "") return;

	para_value_two = "";
	para_value_one = "";

	cout<<current_line<<endl;

	const char *loc = current_line.c_str();
	cout<<endl;

	bool sname = true;
	bool decision_start = false;
	bool decision_end = false;


	bool element_start = false;
	bool element_end = false;
	int  element_index = 0;
	int i = 0;
	for(; i < strlen(loc); i++){
//		cout<<" #"<<i<<" "<<loc[i]<<"#";
/*

		bool spara_name = (loc[i] > 'a' - 1 && loc[i] < 'z' + 1);
		bool spara_value = (loc[i] > '0' - 1 && loc[i] < '9' + 1) || spara_name;
		bool epara_name = (loc[i] < 'a' || loc[i] > 'z') && (loc[i] != '_');
		bool epara_value = (loc[i] < '0' || loc[i] > '9') && (loc[i] != '.') && epara_name;
*/

		bool spara_name = (loc[i] > 'a' - 1 && loc[i] < 'z' + 1) || (loc[i] > 'A' - 1 && loc[i] < 'Z' + 1);
		bool spara_value = (loc[i] > '0' - 1 && loc[i] < '9' + 1) || spara_name;
		bool epara_name = (loc[i] < 'a' || loc[i] > 'z') && (loc[i] != '_') && (loc[i] < 'A' || loc[i] > 'Z');
		bool epara_value = (loc[i] < '0' || loc[i] > '9') && (loc[i] != '.') && epara_name;

		decision_start = sname?spara_name:spara_value;
		decision_end = sname?epara_name:epara_value;

/*

		bool spara_name = loc[i] > 'a' - 1 && loc[i] < 'z' + 1;
		bool spara_value = (loc[i] > '0' - 1 && loc[i] < '9' + 1) || spara_name;
		bool epara_name = (loc[i] < 'a' || loc[i] > 'z') && (loc[i] != '_');
		bool epara_value = (loc[i] < '0' || loc[i] > '9') && (loc[i] != '.') && epara_name;
*/

		if(!element_start && decision_start){
//		if(!element_start && (spara_name || spara_value)){
			element_index = i;
			element_start = true;
			element_end = false;
		}else if(element_start && decision_end){
//		}else if(element_start && (epara_name || epara_value)){
			if(!element_end){
				if(sname){//parameter name part
					cout<<endl;
					cout<<"I'm a name "<<current_line.substr(element_index,i - element_index)<<endl;
					para_name = current_line.substr(element_index,i - element_index);
					sname = !sname;
				}else{//parameter value part
					cout<<"I'm a min value "<<current_line.substr(element_index,i - element_index)<<endl;
					para_value_two = current_line.substr(element_index,i - element_index);
				}
				element_end = true;
				element_start = false;
			}

		}

	}
	cout<<"I'm a normal(max) value "<<current_line.substr(element_index,i - element_index)<<endl;
	para_value_one = current_line.substr(element_index,i - element_index);
}

void
ScenGene::parseValue(int type, int index){
	switch(type){
	case 2:
		nodePara(index);
		break;
	case 3:
		edgePara(index);
		break;
	case 4:
		routePara(index);
		break;
	case 5:
		filePara(index);
		break;
	case 6:
		wiredPara(index);
		break;
	}
}

void
ScenGene::nodePara(int index){
	//hard codes
	switch(index){
	case 0:
		node_prefix = para_value_one;break;
	case 1:
		node_rowc = atoi(para_value_one.c_str());break;
	case 2:
		node_colc = atoi(para_value_one.c_str());break;
	case 3:
		scen_xlength = atof(para_value_one.c_str());break;
	case 4:
		scen_ylength = atof(para_value_one.c_str());break;
	case 5:
		node_axis_type_bits = atoi(para_value_one.c_str());break;
	case 6:
		node_radius = atof(para_value_one.c_str());break;
	case 7:
		node_xstart = atoi(para_value_one.c_str());break;
	case 8:
		node_ystart = atof(para_value_one.c_str());break;
	case 9:
		node_base_station_start = atoi(para_value_one.c_str());break;
	case 10:
		node_junc_type = atoi(para_value_one.c_str());break;
	case 11:
		node_junc_width = atof(para_value_one.c_str());break;
	default:
		cout<<" ERROR exists in nodePara..."<<endl;
	}

	cout<<endl;
	//soft codes
	if(para_value_one.compare(key_node[0]) == 0){
		cout<<"k= "<<key_node[0]<<" v="<<para_value_one<<endl;
	}else{
		cout<<"k= "<<key_node[index]<<" v="<<para_value_one<<endl;
	}
}

void
ScenGene::edgePara(int index){
	//hard codes
	switch(index){
	case 0:
		edge_lane_num = atoi(para_value_one.c_str());break;
	case 1:
		edge_bidirection_bits = atoi(para_value_one.c_str());break;
	case 2:
		edge_connect_bits = atoi(para_value_one.c_str());break;
	case 3:
		edge_prefix = para_value_one;break;
	default:
		cout<<" ERROR exists in edgePara..."<<endl;
	}

	cout<<endl;
	//soft codes
	cout<<"k= "<<key_edge[index]<<" v="<<para_value_one<<endl;

/*	if(para_value_one.compare(key_node[0]) == 0){
		cout<<"k= "<<key_node[0]<<" v="<<para_value_one<<endl;
	}else{
		cout<<"k= "<<key_node[index]<<" v="<<para_value_one<<endl;
	}*/
}

void
ScenGene::routePara(int index){
	//hard codes
	switch(index){
	case 0:
		route_vtypes_prefix = para_value_one;break;
	case 1:
		route_vtypes_num = atoi(para_value_one.c_str());break;
	case 2:
		route_vtypes_accel[0] = atof(para_value_one.c_str());
		route_vtypes_accel[1] = atof(para_value_two.c_str());
		break;
	case 3:
		route_vtypes_decel[0] = atof(para_value_one.c_str());
		route_vtypes_decel[1] = atof(para_value_two.c_str());
		break;
	case 4:
		route_vtypes_length[0] = atoi(para_value_one.c_str());
		route_vtypes_length[1] = atoi(para_value_two.c_str());
		break;
	case 5:
		route_vtypes_maxspeed = atof(para_value_one.c_str());break;
	case 6:
		route_vtypes_width[0] = atoi(para_value_one.c_str());
		route_vtypes_width[1] = atoi(para_value_two.c_str());
		break;
	case 7:
		route_vtypes_sigma[0] = atof(para_value_one.c_str());
		route_vtypes_sigma[1] = atof(para_value_two.c_str());
		break;
	case 8:
		route_trip_prefix = para_value_one;break;
	case 9:
		route_trip_start = atoi(para_value_one.c_str());break;
	case 10:
		route_trip_num = atoi(para_value_one.c_str());break;
	case 11:
		route_edge_range_base = atoi(para_value_one.c_str());break;
	case 12:
		route_edge_range_max = atoi(para_value_one.c_str());break;
	case 13:
		route_edge_start =para_value_one;break;
	case 14:
		route_vehi_prefix = atoi(para_value_one.c_str());break;
	case 15:
		route_vehi_num = atoi(para_value_one.c_str());break;
	case 16:
		route_vehi_types = atoi(para_value_one.c_str());break;
	case 17:
		route_vehi_routes = atoi(para_value_one.c_str());break;
	case 18:
		route_vehi_depart = atoi(para_value_one.c_str());break;
	case 19:
		route_vehi_color_bits = atoi(para_value_one.c_str());break;
	case 20:
		lambda = atoi(para_value_one.c_str());break;
	case 21:
		route_trip_divide_num = atoi(para_value_one.c_str());break;
	default:
		cout<<" ERROR exists in routePara..."<<endl;
	}

	cout<<endl;
	//soft codes
	cout<<"k= "<<key_route[index]<<" v="<<para_value_one<<endl;
}

void
ScenGene::filePara(int index){
	//hard codes
	std::ostringstream strtmp;

	switch(index){
	case 0:
		fileName = para_value_one;break;
	case 1:

		strtmp<<fileName<<"."<<para_value_one;
		nodeFile = strtmp.str();
		break;
	case 2:

		strtmp<<fileName<<"."<<para_value_one;
		edgeFile = strtmp.str();
		break;
	case 3:

		strtmp<<fileName<<"."<<para_value_one;
		routeFile = strtmp.str();
		break;
	case 4:

		strtmp<<fileName<<"."<<para_value_one;
		baseFile = strtmp.str();
		break;
	case 5:

		strtmp<<fileName<<"."<<para_value_one;
		wiredFile = strtmp.str();
		break;
	case 6:

		strtmp<<fileName<<"."<<para_value_one;
		patternFile = strtmp.str();
		break;
	default:
		cout<<" ERROR exists in filePara..."<<endl;
	}

	cout<<endl;
	//soft codes
	cout<<"k= "<<key_file[index]<<" v="<<para_value_one<<endl;
}

void
ScenGene::wiredPara(int index){
	//hard codes
	std::ostringstream strtmp;

	switch(index){
	case 0:
		wired_junc_only = atoi(para_value_one.c_str());break;
	case 1:
		wired_xdist_base = atof(para_value_one.c_str());break;
	case 2:
		wired_ydist_base = atof(para_value_one.c_str());break;
	default:
		cout<<" ERROR exists in wiredPara..."<<endl;
	}

	cout<<endl;
	//soft codes
	cout<<"k= "<<key_wired[index]<<" v="<<para_value_one<<endl;
}
void
ScenGene::printPara(){
	cout<<endl;
	cout<<"---------  ** ** ** ----------------"<<endl;
	cout<<"node parameters lists:"<<endl;
	for(int i = 0; i < 11; i++){
		cout<<"**\t"<<key_node[i]<<" ";
		switch(i){
		case 0:
			cout<<node_prefix;break;
		case 1:
			cout<<node_rowc;break;
		case 2:
			cout<<node_colc;break;
		case 3:
			cout<<scen_xlength;break;
		case 4:
			cout<<scen_ylength;break;
		case 5:
			cout<<node_axis_type_bits;break;
		case 6:
			cout<<node_radius;break;
		case 7:
			cout<<node_xstart;break;
		case 8:
			cout<<node_ystart;break;
		case 9:
			cout<<node_base_station_start;break;
		case 10:
			cout<<node_junc_type;break;
		case 11:
			cout<<node_junc_width;break;
		default:
			cout<<" ERROR exists in node printPara..."<<endl;
		}
		cout<<endl;
	}
	cout<<endl;
	cout<<"edge parameters lists:"<<endl;
	for(int i = 0; i < 4; i++){
		cout<<"**\t"<<key_edge[i]<<" ";
		switch(i){
		case 0:
			cout<<edge_lane_num;break;
		case 1:
			cout<<edge_bidirection_bits;break;
		case 2:
			cout<<edge_connect_bits;break;
		case 3:
			cout<<edge_prefix;break;
		default:
			cout<<" ERROR exists in edge printPara..."<<endl;
		}
		cout<<endl;
	}
	cout<<endl;
	cout<<"route parameters lists:"<<endl;
	for(int i = 0; i < 22; i++){
		cout<<"**\t"<<key_route[i]<<" ";
		switch(i){
		case 0:
			cout<<route_vtypes_prefix;break;
		case 1:
			cout<<route_vtypes_num;break;
		case 2:
			cout<<"maxi value="<<route_vtypes_accel[0];
			cout<<"mini value="<<route_vtypes_accel[1];
			break;
		case 3:
			cout<<"maxi value="<<route_vtypes_decel[0];
			cout<<"mini value="<<route_vtypes_decel[1];
			break;
		case 4:
			cout<<"maxi value="<<route_vtypes_length[0];
			cout<<"mini value="<<route_vtypes_length[1];
			break;
		case 5:
			cout<<route_vtypes_maxspeed;
			break;
		case 6:
			cout<<"maxi value="<<route_vtypes_width[0];
			cout<<"mini value="<<route_vtypes_width[1];
			break;
		case 7:
//			cout<<route_vtypes_sigma;
			cout<<"maxi value="<<route_vtypes_sigma[0];
			cout<<"mini value="<<route_vtypes_sigma[1];
			break;
		case 8:
			cout<<route_trip_prefix;
			break;
		case 9:
			cout<<route_trip_start;
			break;
		case 10:
			cout<<route_trip_num;
			break;
		case 11:
			cout<<route_edge_range_base;
			break;
		case 12:
			cout<<route_edge_range_max;
			break;
		case 13:
			cout<<route_edge_start;
			break;
		case 14:
			cout<<route_vehi_prefix;
			break;
		case 15:
			cout<<route_vehi_num;
			break;
		case 16:
			cout<<route_vehi_types;
			break;
		case 17:
			cout<<route_vehi_routes;
			break;
		case 18:
			cout<<route_vehi_depart;
			break;
		case 19:
			cout<<route_vehi_color_bits;break;
		case 20:
			cout<<lambda;break;
		case 21:
			cout<<route_trip_divide_num;break;
		default:
			cout<<" ERROR exists in route printPara..."<<endl;

		}
		cout<<endl;
	}

	cout<<endl;
	cout<<"file parameters lists:"<<endl;
	for(int i = 0; i < 7; i++){
		cout<<"**\t"<<key_file[i]<<" ";
		switch(i){
		case 0:
			cout<<fileName;break;
		case 1:
			cout<<nodeFile;break;
		case 2:
			cout<<edgeFile;break;
		case 3:
			cout<<routeFile;break;
		case 4:
			cout<<baseFile;break;
		case 5:
			cout<<wiredFile;break;
		case 6:
			cout<<patternFile;break;
		default:
			cout<<" ERROR exists in file printPara..."<<endl;
		}
		cout<<endl;
	}

	cout<<endl;
	cout<<"wired parameters lists:"<<endl;
	for(int i = 0; i < 3; i++){
		cout<<"**\t"<<key_wired[i]<<" ";
		switch(i){
		case 0:
			cout<<wired_junc_only;break;
		case 1:
			cout<<wired_xdist_base;break;
		case 2:
			cout<<wired_ydist_base;break;
		default:
			cout<<" ERROR exists in wired printPara..."<<endl;
		}
		cout<<endl;
	}
}

void
ScenGene::readconf(){

//	string finput_name = "scenParameter.conf";


	cout<<"\n*****--------------------------------******"<<endl;
	cout<<"Please make sure that your configure file exists in the current directory"<<endl;
	cout<<endl;
	cout<<"\tInput file name------scenParameter.conf"<<endl;
	cout<<"the number of available parameters IS more than 30.!!!!"<<endl;
	cout<<"Note, each valid line start with \"**\" end with \"++\""<<endl;
	cout<<"\n*****--------------------------------******"<<endl;

	ifstream paraFile(confFile.c_str(),ios_base::in);//read only

	if(paraFile.fail()){
		cout<<"scenParameter.conf   OPEN   failed"<<endl;
	}else{
		cout<<"Content are....."<<endl;
		string cont;
		bool para_start = false;
		int  Enter_index = 1;
		int  element_index = -1;
		while(getline(paraFile,cont)){
			if(cont.find(key_Input[0].c_str())){
				if(!cont.find(key_Input[1].c_str())){
					element_index++;
				}else{
					continue;//invalid line of input
				}
			}else{
//				cout<<"\n new entrance..."<<endl;
				para_start = !para_start;
				Enter_index++;
				element_index = -1;
				continue;
			}

			cout<<"\n================\ncurrent conf is "<<key_Input[Enter_index]<<" parameter is ";

			switch(Enter_index){
			case 2://  node parameters..................
				cout<<" "<<key_node[element_index]<<endl;
				current_line = cont;
				parseLine(Enter_index,element_index);
				parseValue(Enter_index,element_index);
				break;
			case 3://  edge parameters..................
				cout<<" "<<key_edge[element_index]<<endl;
				current_line = cont;
				parseLine(Enter_index,element_index);
				parseValue(Enter_index,element_index);
				break;
			case 4://  route parameters..................
				cout<<" "<<key_route[element_index]<<endl;
				current_line = cont;
				parseLine(Enter_index,element_index);
				parseValue(Enter_index,element_index);
				break;
			case 5://  file parameters..................
				cout<<" "<<key_file[element_index]<<endl;
				current_line = cont;
				parseLine(Enter_index,element_index);
				parseValue(Enter_index,element_index);
				break;
			case 6://  wired parameters..................
				cout<<" "<<key_wired[element_index]<<endl;
				current_line = cont;
				parseLine(Enter_index,element_index);
				parseValue(Enter_index,element_index);
				break;
			default:
				cout<<" \n\n\t----ERROR in scenParameter.conf--------"<<key_Input[Enter_index]<<"--part"<<endl;

				break;
			}
		}
	}

	paraFile.close();

	printPara();


	base_loc[0] = new double[node_rowc * node_colc + 1];
	base_loc[1] = new double[node_rowc * node_colc + 1];
	base_loc[2] = new double[node_rowc * node_colc + 1];

}


void
ScenGene::IntToChars(int index){
	int ri = index;
	if (ri > 999) {
		printf(" rowno_next ERROR to string");
	}else if (ri > 99) {
		printf("%c%c%c",digits[ri / 100],digits[(ri % 100) / 10],digits[ri % 10]);
//		file_handle<<digits[ri / 100]<<digits[(ri % 100) / 10]<<digits[ri % 10];
	}else if (ri > 9) {
		printf("%c%c%c",digits[0],digits[(ri % 100) / 10],digits[ri % 10]);
//		file_handle<<digits[0]<<digits[(ri % 100) / 10]<<digits[ri % 10];
	}else{
		printf("%c%c%c",digits[0],digits[0],digits[ri % 10]);
//		file_handle<<digits[0]<<digits[0]<<digits[ri % 10];

	}

}


void
ScenGene::IntToChars(int index, char* cont){
	int ri = index;
	if (ri > 999) {
		printf(" rowno_next ERROR to string");
	}else if (ri > 99) {
		printf("%c%c%c",digits[ri / 100],digits[(ri % 100) / 10],digits[ri % 10]);
		cont[0] = digits[ri / 100];
		cont[1] = digits[(ri % 100) / 10];
		cont[2] = digits[ri % 10];
//		cont<<digits[ri / 100]<<digits[(ri % 100) / 10]<<digits[ri % 10];
//		file_handle<<digits[ri / 100]<<digits[(ri % 100) / 10]<<digits[ri % 10];
	}else if (ri > 9) {
		printf("%c%c%c",digits[0],digits[(ri % 100) / 10],digits[ri % 10]);
		cont[0] = digits[0];
		cont[1] = digits[(ri % 100) / 10];
		cont[2] = digits[ri % 10];
//		cont<<digits[0]<<digits[(ri % 100) / 10]<<digits[ri % 10];
//		file_handle<<digits[0]<<digits[(ri % 100) / 10]<<digits[ri % 10];
	}else{
		printf("%c%c%c",digits[0],digits[0],digits[ri % 10]);
		cont[0] = digits[0];
		cont[1] = digits[0];
		cont[2] = digits[ri % 10];
//		cont<<digits[0]<<digits[0]<<digits[ri % 10];
//		file_handle<<digits[0]<<digits[0]<<digits[ri % 10];

	}

}


void
ScenGene::TwoIntToChars(int index){
	int ri = index;
	if (ri > 99) {
		printf(" rowno_next ERROR to string");
	}else if (ri > 9) {
		printf("%c%c",digits[(ri % 100) / 10],digits[ri % 10]);
//		file_handle<<digits[(ri % 100) / 10]<<digits[ri % 10];
	}else{
		printf("%c%c",digits[0],digits[ri % 10]);
//		file_handle<<digits[0]<<digits[ri % 10];

	}
}

void
ScenGene::TwoIntToChars(int index, char* cont){
	int ri = index;
	if (ri > 99) {
		printf(" rowno_next ERROR to string");
	}else if (ri > 9) {
		printf("%c%c",digits[(ri % 100) / 10],digits[ri % 10]);
//		file_handle<<digits[(ri % 100) / 10]<<digits[ri % 10];
	}else{
		printf("%c%c",digits[0],digits[ri % 10]);
//		file_handle<<digits[0]<<digits[ri % 10];

	}
}


void
ScenGene::file_nodxml(){
	double xstep = 0.0;
	double ystep = 0.0;
//	double zstep = 0.0;
	double xstart = node_xstart;
	double ystart = node_ystart;
	double Xlength = scen_xlength - xstart * 2;
	double Ylength = scen_ylength - ystart * 2;

	int	   rowC = node_rowc;
	int    colC = node_colc;
	string   prefix = node_prefix;
	int    node_ttttype = node_junc_type;
	double	radius = node_junc_width;

	printf("\n junction info list as follows:\n\n<nodes>\n");

//	file_handle<<"<nodes>\n"<<endl;
	std::ostringstream strtmp;
	char* icStr = new char[4];

	strtmp<<"<nodes>\n";

	int   idcharLen = 6;

//	char  tmp_Result[(rowC * colC + rowC) * idcharLen + 1];

	if(node_axis_type_bits == 7){// x same step, y same step
		xstep = Xlength / (colC - 1);
		ystep = Ylength / (rowC - 1);


		//for base information
		base_xstep = xstep;
		base_ystep = ystep;

//		printf("...............%d.....%d....\n", rowC, colC);
		int ri = 1;
		for(; ri < rowC + 1; ri++){
//			printf(".....++ ++.......%d.....%d....\n", ri, colC);
			int cj = 1;
			for(; cj < colC + 1; cj++){
//				printf("....** **.......%d.....%d....\n", ri, cj);
				printf("\n<node id=\"%s",prefix.c_str());
				strtmp<<"\n<node id=\""<<prefix.c_str();
//				file_handle<<"\n<node id=\""<<prefix;
//				cout<<"\n<node id=\""<<prefix;

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
//				IntToChars(ri);

//				IntToChars(cj);
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				//x y
				printf("\" x=\"%0.2f\" y=\"%0.2f",(xstart + xstep * cj - xstep),(ystart + ystep * ri - ystep));
//				file_handle<<"\" x=\""<<(xstart + xstep * cj - xstep)<<"\" y=\""<<(ystart + ystep * ri - ystep);
				strtmp<<"\" x=\""<<(xstart + xstep * cj - xstep)<<"\" y=\""<<(ystart + ystep * ri - ystep);

						//base location
						base_loc[0][(ri - 1) * colC + cj] = (xstart + xstep * cj - xstep);
						base_loc[1][(ri - 1) * colC + cj] = (ystart + ystep * ri - ystep);
						base_loc[2][(ri - 1) * colC + cj] = 0.0;
//				printf("\" bx=\"%0.2f\" bi=\"%d\"",base_loc[0][(ri - 1) * colC + cj],(ri - 1) * colC + cj);

				//nodetype
				printf("\" type=\"%s",nodeTypeStr[node_ttttype].c_str());
//				file_handle<<" type=\""<<nodeTypeStr[node_ttttype];
				strtmp<<"\" type=\""<<nodeTypeStr[node_ttttype];
				//radius
				printf("\" radius=\"%0.2f",radius);
//				file_handle<<" radius=\""<<radius;
				strtmp<<"\" radius=\""<<radius;



				printf("\" />");
//				file_handle<<" />";
				strtmp<<"\" />";
			}
		}

		printf("\n</nodes>");
//		file_handle<<"\n</nodes>";
		strtmp<<"\n</nodes>";

	}else if (node_axis_type_bits == 5){// x differential step, y same step
		srand((unsigned)time(NULL));

		int		MAX_D_value = 12;
		int		MAX_M_value = 14;

		double xstart = node_xstart;
		double ystart = node_ystart;

		double step_basex = Xlength / colC / 4;
		double step_basey = Ylength / rowC / 4;

		int    max_xstep = floor(Xlength / (colC - 1));
		int    max_ystep = floor(Ylength / (rowC - 1));

		//to modify the value of sum_x/y
		double checkX = Xlength / (colC - 1);
		double checkY = Ylength / (rowC - 1);

		double sum_Y = ystart;
//		printf("r...............%d.....%d....\n", rowC, colC);
		int ri = 1;
		for(; ri < rowC + 1; ri++){

			double sum_X = xstart;
			int random_Yv = rand() % MAX_M_value;
			random_Yv += 4;
			double rateY = random_Yv * 1.0 / MAX_D_value;

//			printf("r.....++ ++.......%d.....%d....\n", ri, colC);
			int cj = 1;
			for(; cj < colC + 1; cj++){
//				printf("r....** **.......%d.....%d....\n", ri, cj);
				printf("\n<node id=\"%s",prefix.c_str());
				strtmp<<"\n<node id=\""<<prefix.c_str();

//				IntToChars(ri);
				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj);
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				int random_Xv = rand() % MAX_M_value;


				random_Xv += 4;


				double rateX = random_Xv * 1.0 / MAX_D_value;



				//x y
				printf("\" x=\"%0.2f\" y=\"%0.2f\"",sum_X,sum_Y);
				strtmp<<"\" x=\""<<sum_X<<" y=\""<<sum_Y;
				//nodetype
				printf("\" type=\"%s\"",nodeTypeStr[node_ttttype].c_str());
				strtmp<<"\" type=\""<<nodeTypeStr[node_ttttype].c_str();
				//radius
				printf("\" radius=\"%0.2f\"",radius);
				strtmp<<"\" radius=\""<<radius;



				printf("\" />");
				strtmp<<"\" />";

				sum_X = sum_X +  rateX * max_xstep;

				//check x value for standard since random value always < 1.0
				if((checkX * cj + xstart) - (sum_X + 1.5 * step_basex) > 1.0){
					sum_X += step_basex;
				} else if ((sum_X - 1.5 * step_basex) - (checkX * cj + xstart)> 1.0){
					sum_X -= step_basex;
				}

				//complete the location of the last node
				if(cj == colC - 1){
					sum_X = scen_xlength - 5;
				}
			}

			sum_Y = sum_Y + max_ystep * rateY;

			//check y value for standard since random value always < 1.0
			if((checkY * ri + ystart) - (sum_Y + 1.5 * step_basey) > 1.0){
				sum_Y += step_basey;
			} else if ((sum_Y - 1.5 * step_basey) - (checkY * ri + ystart)> 1.0){
				sum_Y -= step_basey;
			}

			//complete the location of the last node
			if(ri == rowC - 1){
				sum_Y = scen_ylength - 7;
			}
		}






		printf("\n</nodes>");
		strtmp<<"\n</nodes>";
	}else if(node_axis_type_bits == 6){// x same step, y differential step

	}else if(node_axis_type_bits == 4){// x differential step, y differential step

	}else{
		printf("\nbad choices, X step, Y step ERROR");
	}

	string contf = strtmp.str();
	nodeXML(contf);
//	file_handle.flush();
//	file_handle.close();
}


void
ScenGene::file_edgxml(){

	int	   rowC = node_rowc;
	int    colC = node_colc;
	string prefix = edge_prefix;

	std::ostringstream strtmp;
	char* icStr = new char[4];
	strtmp<<"<edges>\n";

	printf("\n edge info list as follows:\n\n<edges>\n");

	int ri = 1;
	for(; ri < rowC + 1; ri++){
//			printf(".....++ ++.......%d.....%d....\n", ri, colC);
		int cj = 1;
		for(; cj < colC + 1; cj++){

			int ni = -1;
			int nj = -1;

			ni = ri + 1;
			nj = cj;
			if(ni < rowC + 1){
				strtmp<<"\n<edge id=\""<<prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" from=\""<<node_prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" to=\""<<node_prefix.c_str();
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" numLanes=\""<<edge_lane_num<<"\" > </edge>";

//				edge_id(ri,cj,ni,nj);
//				edge_ft(ri,cj,ni,nj);
//				edge_lanes(edge_lane_num);
//				edge_finishLine();
				//id(ri,cj,ni,nj);
				//ft(ri,cj,ni,nj);
				//lanes(1);
				//finishLine();
			}

			ni = ri - 1;
			nj = cj;

			if(ni > 0){
				strtmp<<"\n<edge id=\""<<prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" from=\""<<node_prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" to=\""<<node_prefix.c_str();
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" numLanes=\""<<edge_lane_num<<"\" > </edge>";

				edge_id(ri,cj,ni,nj);
				edge_ft(ri,cj,ni,nj);
				edge_lanes(edge_lane_num);
				edge_finishLine();
//				id(ri,cj,ni,nj);
//				ft(ri,cj,ni,nj);
//				lanes(1);
//				finishLine();
			}

			ni = ri;
			nj = cj + 1;

			if(nj < colC + 1){
				strtmp<<"\n<edge id=\""<<prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" from=\""<<node_prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" to=\""<<node_prefix.c_str();
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" numLanes=\""<<edge_lane_num<<"\" > </edge>";

				edge_id(ri,cj,ni,nj);
				edge_ft(ri,cj,ni,nj);
				edge_lanes(edge_lane_num);
				edge_finishLine();
//				id(ri,cj,ni,nj);
//				ft(ri,cj,ni,nj);
//				lanes(1);
//				finishLine();
			}

			ni = ri;
			nj = cj - 1;

			if(nj > 0){
				strtmp<<"\n<edge id=\""<<prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" from=\""<<node_prefix.c_str();

				IntToChars(ri,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(cj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" to=\""<<node_prefix.c_str();
				IntToChars(ni,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];
				IntToChars(nj,icStr);
				strtmp<<icStr[0]<<icStr[1]<<icStr[2];

				strtmp<<"\" numLanes=\""<<edge_lane_num<<"\" > </edge>";

				edge_id(ri,cj,ni,nj);
				edge_ft(ri,cj,ni,nj);
				edge_lanes(edge_lane_num);
				edge_finishLine();
//				id(ri,cj,ni,nj);
//				ft(ri,cj,ni,nj);
//				lanes(1);
//				finishLine();
			}


		}
	}

	printf("\n </edges>");
	strtmp<<"\n </edges>";

	string contf = strtmp.str();
	edgeXML(contf);
}

void
ScenGene::file_rouxml(){

	std::ostringstream strtmp;
	srand((unsigned)time(NULL));
	strtmp<<"<routes>\n";

//vehicular types definition
	double vtype_tmp_accel = 0.0;
	double vtype_tmp_decel = 0.0;
	double vtype_tmp_sigma = 0.0;
	int    vtype_tmp_length = 0;
	int    vtype_tmp_width = 0;

	strtmp<<"\n";
	for(int i = 1; i < route_vtypes_num + 1; i++){
		vtype_tmp_accel = ((rand() % 10) * 1.0 / 10) * (route_vtypes_accel[0] - route_vtypes_accel[1]);
		vtype_tmp_accel = vtype_tmp_accel + route_vtypes_accel[1];

/*		vtype_tmp_decel = (rand() % ((int)floor((route_vtypes_decel[0] - route_vtypes_decel[1]) * 1000))) / floor((route_vtypes_decel[0] - route_vtypes_decel[1]) * 1000);
		vtype_tmp_decel = vtype_tmp_decel / 1000;*/
		vtype_tmp_decel = ((rand() % 10) * 1.0 / 10) * (route_vtypes_decel[0] - route_vtypes_decel[1]);
		vtype_tmp_decel = vtype_tmp_decel + route_vtypes_decel[1];

		vtype_tmp_sigma = ((rand() % 10) * 1.0 / 10) * (route_vtypes_sigma[0] - route_vtypes_sigma[1]);
		vtype_tmp_sigma = vtype_tmp_sigma + route_vtypes_sigma[1];

//		vtype_tmp_length = (rand() % (route_vtypes_length[0] - route_vtypes_length[1])) * 1.0 / (route_vtypes_length[0] - route_vtypes_length[1]);
		vtype_tmp_length = (int)(((rand() % 10) * 1.0 / 10) * (route_vtypes_length[0] - route_vtypes_length[1]));
		vtype_tmp_length = vtype_tmp_length + route_vtypes_length[1];

		vtype_tmp_width = (int)(((rand() % 10) * 1.0 / 10) * (route_vtypes_width[0] - route_vtypes_width[1]));
		vtype_tmp_width = vtype_tmp_width + route_vtypes_width[1];

		strtmp<<"<vType id=\""<<route_vtypes_prefix.c_str()<<i<<"\" accel=\""<<vtype_tmp_accel<<"\" decel=\""<<vtype_tmp_decel<<"\" maxSpeed=\""<<route_vtypes_maxspeed;
		strtmp<<"\" length=\""<<vtype_tmp_length<<"\" width=\""<<vtype_tmp_width<<"\" sigma=\""<<vtype_tmp_sigma<<"\" />\n";
	}
	strtmp<<"\n";

//route path definition
	int fromjunc[6];

	char from[12];
	strcpy(from, (node_prefix + route_edge_start).c_str());
	parseJunc(from,fromjunc);


	int row_count = node_rowc;
	int column_count = node_colc;

	int rc = route_trip_num + 1;

	const char right[2] = {'0','1'};



	char tmp_Edge[14];


	char Trips[600][781];// 1 + 13*60

	int Current_trip[500];

	int ti = 1;
	int ei = 1;







	char* icStr = new char[4];

	 printf("\n");
	for(;ti < rc; ti++){// the number of trips

		//random edge start
		if(ti % (route_trip_num / (route_trip_divide_num-1)) == (route_trip_num / (route_trip_divide_num-1)) - 1){
			int tmp_Random_row = rand() % node_rowc + 1;
			int tmp_Random_col = rand() % node_colc + 1;

			fromjunc[0] = tmp_Random_row / 100;
			fromjunc[1] = (tmp_Random_row % 100) / 10;
			fromjunc[2] = tmp_Random_row % 10;

			fromjunc[3] = tmp_Random_col / 100;
			fromjunc[4] = (tmp_Random_col % 100) / 10;
			fromjunc[5] = tmp_Random_col % 10;
		}




		int cc = rand() % route_edge_range_max + route_edge_range_base;

		Trips[ti][0] = cc;//the number of edges

		ei = 1;
		int rowno_next = -1;
		int colno_next = -1;
		for(int tmp_ei = 0; tmp_ei < 240; tmp_ei++){
			Current_trip[tmp_ei*2] = -1;
			Current_trip[tmp_ei*2+1] = -1;
		}



		printf("\n\n\n\n<route id=\"route%d\" color=\"1,1,0\" edges=\"",ti);
		strtmp<<"<route id=\"route"<<ti<<"\" color=\"1,1,0\" edges=\"";
		for(;ei < cc; ei++){// the number of edges
				//the first random road segment


			//fix cycle bug
			bool available[4];

			available[0] = true;
			available[1] = true;
			available[2] = true;
			available[3] = true;

//			printf("\tESi=%d cri=%d, cci=%d",cc,rand()%row_count, rand()%column_count);
			int rowno_index = (ei < 2)?(fromjunc[0]*100 + fromjunc[1]*10 + fromjunc[2]):rowno_next;
			int colno_index = (ei < 2)?(fromjunc[3]*100 + fromjunc[4]*10 + fromjunc[5]):colno_next;

			Current_trip[ei*2-1] = rowno_index;
			Current_trip[ei*2] = colno_index;

//			printf("junction is ri=%d, ci=%d\t",rowno_index,  colno_index);
			bool valid_next = false;

			int choose_direction;

			available[0] = check_valid(rowno_index, colno_index, 0);
			available[1] = check_valid(rowno_index, colno_index, 1);
			available[2] = check_valid(rowno_index, colno_index, 2);
			available[3] = check_valid(rowno_index, colno_index, 3);



			while(!valid_next){
				choose_direction = rand() % 4;
				while(!check_valid(rowno_index, colno_index, choose_direction))
					choose_direction = rand() % 4;
				switch(choose_direction){//choose the direction
				case 0:
//					printf("next junction is nri=%d, nci=%d",(rowno_index-1),  colno_index);
					rowno_next = rowno_index-1;
					colno_next = colno_index;
					break;
				case 1:
//					printf("next junction is nri=%d, nci=%d", rowno_index   , (colno_index+1));
					rowno_next = rowno_index;
					colno_next = colno_index+1;
					break;
				case 2:
//					printf("next junction is nri=%d, nci=%d",(rowno_index+1), colno_index);
					rowno_next = rowno_index+1;
					colno_next = colno_index;
					break;
				case 3:
//					printf("next junction is nri=%d, nci=%d", rowno_index   , (colno_index-1));
					rowno_next = rowno_index;
					colno_next = colno_index-1;
					break;
				default:
					printf("ERROR!!!!!!!!!"); break;
				}

				int collision = 0;
//				printf("\n");
				for(int tmp_ei = 1; tmp_ei < ei + 1; tmp_ei++){
					char rowChar[6]={'1'};
					char colChar[6]={'1'};
					if (Current_trip[tmp_ei*2-1] == rowno_next){
						rowChar[0] = 't';
						rowChar[1] = 'r';
						rowChar[2] = 'u';
						rowChar[3] = 'e';
						rowChar[4] = '\0';
						if (Current_trip[tmp_ei*2] == colno_next){
							collision++;
							colChar[0] = 't';
							colChar[1] = 'r';
							colChar[2] = 'u';
							colChar[3] = 'e';
							colChar[4] = '\0';
						}else{
							colChar[0] = 'f';
							colChar[1] = 'a';
							colChar[2] = 'l';
							colChar[3] = 's';
							colChar[4] = 'e';
							colChar[5] = '\0';
						}
					}else{
						rowChar[0] = 'f';
						rowChar[1] = 'a';
						rowChar[2] = 'l';
						rowChar[3] = 's';
						rowChar[4] = 'e';
						rowChar[5] = '\0';
						if (Current_trip[tmp_ei*2] == colno_next){
							colChar[0] = 't';
							colChar[1] = 'r';
							colChar[2] = 'u';
							colChar[3] = 'e';
							colChar[4] = '\0';
						}else{
							colChar[0] = 'f';
							colChar[1] = 'a';
							colChar[2] = 'l';
							colChar[3] = 's';
							colChar[4] = 'e';
							colChar[5] = '\0';
						}
					}
//					printf("\t-- %d %d %d %d %s %s",Current_trip[tmp_ei*2-1],Current_trip[tmp_ei*2],rowno_next,colno_next,rowChar,colChar);
				}
//				printf("ti=%d  ei=%d collision=%d ",ti, ei,collision);
				if (collision < 1) valid_next = true;
				else{
					available[choose_direction] = false;

					if(!available[0] && !available[1] && !available[2] && !available[3]){
/*						rowno_index = 1000;
						colno_index = 1000;
						rowno_next = 1000;
						colno_next = 1000;*/
//						printf("\n\n\t--- ***** ERROR ***** ---\n\n");
						ei = cc + 10;
//						ti = rc + 10;

						break;
					}
				}
			}
			if(ei < cc){//fix cycle bug
				Trips[ti][ei*13+1] = edge_prefix.c_str()[0];
				//construct edge string

				//row_index
				if (rowno_index > 999) {
					printf(" rowno_next ERROR to string");
				}else if (rowno_index > 99) {
					Trips[ti][ei*13+2] = digits[rowno_index / 100];
					Trips[ti][ei*13+3] = digits[(rowno_index % 100) / 10];
					Trips[ti][ei*13+4] = digits[rowno_index % 10];
				}else if (rowno_index > 9) {
					Trips[ti][ei*13+2] = digits[0];
					Trips[ti][ei*13+3] = digits[(rowno_index % 100) / 10];
					Trips[ti][ei*13+4] = digits[rowno_index % 10];
				}else{
					Trips[ti][ei*13+2] = digits[0];
					Trips[ti][ei*13+3] = digits[0];
					Trips[ti][ei*13+4] = digits[rowno_index % 10];
				}
				//col_index
				if (colno_index > 999) {
					printf(" rowno_next ERROR to string");
				}else if (colno_index > 99) {
					Trips[ti][ei*13+5] = digits[colno_index / 100];
					Trips[ti][ei*13+6] = digits[(colno_index % 100) / 10];
					Trips[ti][ei*13+7] = digits[colno_index % 10];
				}else if (colno_index > 9) {
					Trips[ti][ei*13+5] = digits[0];
					Trips[ti][ei*13+6] = digits[(colno_index % 100) / 10];
					Trips[ti][ei*13+7] = digits[colno_index % 10];
				}else{
					Trips[ti][ei*13+5] = digits[0];
					Trips[ti][ei*13+6] = digits[0];
					Trips[ti][ei*13+7] = digits[colno_index % 10];
				}

				//row_next
				if (rowno_next > 999) {
					printf(" rowno_next ERROR to string");
				}else if (rowno_next > 99) {
					Trips[ti][ei*13+8] = digits[rowno_next / 100];
					Trips[ti][ei*13+9] = digits[(rowno_next % 100) / 10];
					Trips[ti][ei*13+10] = digits[rowno_next % 10];
				}else if (rowno_next > 9) {
					Trips[ti][ei*13+8] = digits[0];
					Trips[ti][ei*13+9] = digits[(rowno_next % 100) / 10];
					Trips[ti][ei*13+10] = digits[rowno_next % 10];
				}else{
					Trips[ti][ei*13+8] = digits[0];
					Trips[ti][ei*13+9] = digits[0];
					Trips[ti][ei*13+10] = digits[rowno_next % 10];
				}
				//col_next
				if (colno_next > 999) {
					printf(" rowno_next ERROR to string");
				}else if (colno_next > 99) {
					Trips[ti][ei*13+11] = digits[colno_next / 100];
					Trips[ti][ei*13+12] = digits[(colno_next % 100) / 10];
					Trips[ti][ei*13+13] = digits[colno_next % 10];
				}else if (colno_next > 9) {
					Trips[ti][ei*13+11] = digits[0];
					Trips[ti][ei*13+12] = digits[(colno_next % 100) / 10];
					Trips[ti][ei*13+13] = digits[colno_next % 10];
				}else{
					Trips[ti][ei*13+11] = digits[0];
					Trips[ti][ei*13+12] = digits[0];
					Trips[ti][ei*13+13] = digits[colno_next % 10];
				}

				int edge_i = 0;
				if(ei > 1){
					printf(" ");
					strtmp<<" ";
				}
				for(; edge_i < 13; edge_i++){
					printf("%c",Trips[ti][ei*13+1+edge_i]);
					strtmp<<Trips[ti][ei*13+1+edge_i];
				}
//				printf("\n ---    ****     -----\n");
			}

		}
		printf("\" />");
		strtmp<<"\" /> \n";
	}
	printf("\n");
	strtmp<<"\n";


//vehicle binding with route
//	srand((unsigned)time(NULL));
	int delay[route_vehi_num+1];
	generatePoisson(delay);
	for(int ri = 1; ri < route_vehi_num + 1; ri++){

		int vType = rand() % route_vehi_types + 1;
		int rType = rand() % route_trip_num + 1;

		double rColor = ((rand() % 10) * 1.0) / 10;
		double gColor = ((rand() % 10) * 1.0) / 10;
		double bColor = ((rand() % 10) * 1.0) / 10;

		strtmp<<"\n<vehicle id=\""<<ri<<"\" type=\"type"<<vType<<"\" route=\"route"<<rType<<"\" depart=\""<<delay[ri];
		strtmp<<"\" color=\""<<rColor<<","<<gColor<<","<<bColor<<"\" />";

		printf("\n<vehicle id=\"%d",ri);
		printf("\" type=\"type%d",vType);
		printf("\" route=\"route%d",rType);
		printf("\" depart=\"%d",delay[ri]);
		printf("\" color=\"%0.1f,%0.1f,%0.1f",rColor,gColor,bColor);
		printf("\" />");
	}

	strtmp<<"\n</routes>";
	string contf = strtmp.str();
	routeXML(contf);
}

void
ScenGene::file_bastcl(){

	base_startIndex = route_vehi_num;//NO. is start from the id of the vehicular

	std::ostringstream strtmp;
	strtmp<<"\n";

	vehibases_num = route_vehi_num;


//	if(base_xstep < node_radius * 2 + 10.0 || base_ystep < node_radius * 2 + 10.0) return;


			int Base_rowc = node_rowc;
			int Base_colc = node_colc;
			int Base_Num = Base_rowc*Base_colc;

			vehibases_num += Base_Num;

			int Between_XNum = floor((base_xstep - node_radius * 2) / node_radius / 2);

			int Between_YNum = floor((base_ystep - node_radius * 2) / node_radius / 2);

			double base_cxstep = 0.0;
			double base_cystep = 0.0;

			if(Between_XNum > 0)base_cxstep = (base_xstep) / (Between_XNum + 1); //since we want the edges, which is more than the no. of junctions
			if(Between_YNum > 0)base_cystep = (base_ystep) / (Between_YNum + 1);

	//		int row_Num = Between_XNum * (COL_COUNT - 1); //since we want the edges, which is less than the no. of junctions
	//		int col_Num = Between_YNum * (ROW_COUNT - 1);

			printf("the xno.=%d, yno.=%d, and interval=%0.2f, orignInterval=%0.2f\n\n",Between_XNum,Between_YNum,base_cxstep,base_xstep);



			for(int ri = 1; ri < Base_Num + 1; ri++){
				printf("\nset node_(");
				printf("%d",(base_startIndex + ri - 1));
				printf(") [$ns_ node]\n");

//				strtmp<<"\nset node_("<<(base_startIndex + ri - 1)<<") [$ns_ node]\n";

				printf("$node_(");
				printf("%d",(base_startIndex + ri - 1));
				printf(") set X_ %0.4f\n",base_loc[0][ri]);

				strtmp<<"$node_("<<(base_startIndex + ri - 1)<<") set X_ "<<base_loc[0][ri]<<"\n";

				printf("$node_(");
				printf("%d",(base_startIndex + ri - 1));
				printf(") set Y_ %0.4f\n",base_loc[1][ri]);

				strtmp<<"$node_("<<(base_startIndex + ri - 1)<<") set Y_ "<<base_loc[1][ri]<<"\n";

				printf("$node_(");
				printf("%d",(base_startIndex + ri - 1));
				printf(") set Z_ %0.4f\n",base_loc[2][ri]);

				strtmp<<"$node_("<<(base_startIndex + ri - 1)<<") set Z_ "<<base_loc[2][ri]<<"\n";

				printf("$node_(");
				printf("%d",(base_startIndex + ri - 1));
				printf(") color \"blue\"\n");

				strtmp<<"$node_("<<(base_startIndex + ri - 1)<<") color \"blue\"\n";
			}


			int New_Index = 0;

			for (int ri = 1; ri < Base_rowc; ri++) { //last row NOT add junction to next row

				for (int cj = 1; cj < Base_colc; cj++) { //last column NOT add junction to next column

					for(int bi = 1; bi < Between_XNum + 1; bi++){//extend with adding junctions to next column

						New_Index++;

						printf("\nset node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") [$ns_ node]\n");

//						strtmp<<"\nset node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") [$ns_ node]\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") set X_ %0.4f\n",base_loc[0][(ri - 1) * Base_colc + cj] + bi * base_cxstep);

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") set X_ "<<base_loc[0][(ri - 1) * Base_colc + cj] + bi * base_cxstep<<"\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") set Y_ %0.4f\n",base_loc[1][(ri - 1) * Base_colc + cj]);

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") set Y_ "<<base_loc[1][(ri - 1) * Base_colc + cj]<<"\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") set Z_ %0.4f\n",base_loc[2][(ri - 1) * Base_colc + cj]);

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") set Z_ "<<base_loc[2][(ri - 1) * Base_colc + cj]<<"\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") color \"red\"\n");

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") color \"red\"\n";

					}

					for(int bi = 1; bi < Between_YNum + 1; bi++){//extend with adding junctions to next row

						New_Index++;

						printf("\nset node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") [$ns node]\n");

//						strtmp<<"\nset node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") [$ns_ node]\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") set X_ %0.4f\n",base_loc[0][(ri - 1) * Base_colc + cj]);

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") set X_ "<<base_loc[0][(ri - 1) * Base_colc + cj]<<"\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") set Y_ %0.4f\n",base_loc[1][(ri - 1) * Base_colc + cj] + bi * base_cystep);

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") set Y_ "<<base_loc[1][(ri - 1) * Base_colc + cj] + bi * base_cystep<<"\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") set Z_ %0.4f\n",base_loc[2][(ri - 1) * Base_colc + cj]);

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") set Z_ "<<base_loc[2][(ri - 1) * Base_colc + cj]<<"\n";

						printf("$node_(");
						printf("%d",(base_startIndex + Base_Num - 1 + New_Index));
						printf(") color \"red\"\n");

						strtmp<<"$node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") color \"red\"\n";

					}

				}

	/*			//right-orient node
				printf("\nset node_(2");
				TwoIntToChars(((ri - 1) / COL_COUNT + 1));
				TwoIntToChars(((ri - 1) % COL_COUNT + 1));*/

			}

			string contf = strtmp.str();
			baseStationTCL(contf);

			vehibases_num += New_Index;
			printf("\n\nthe amout of base stations is %d", New_Index);


}

void
ScenGene::file_wirtcl(){
	int wired_startIndex = vehibases_num;//NO. is start from the id of the vehicular

	std::ostringstream strtmp;
	strtmp<<"\n";

//	if(base_xstep < node_radius * 2 + 10.0 || base_ystep < node_radius * 2 + 10.0) return;


			int Base_rowc = node_rowc;
			int Base_colc = node_colc;
			int Base_Num = Base_rowc*Base_colc;

			int Between_XNum = floor((base_xstep - node_radius * 2) / node_radius / 2);

			int Between_YNum = floor((base_ystep - node_radius * 2) / node_radius / 2);

			double base_cxstep = 0.0;
			double base_cystep = 0.0;

			if(Between_XNum > 0)base_cxstep = (base_xstep) / (Between_XNum + 1); //since we want the edges, which is more than the no. of junctions
			if(Between_YNum > 0)base_cystep = (base_ystep) / (Between_YNum + 1);

	//		int row_Num = Between_XNum * (COL_COUNT - 1); //since we want the edges, which is less than the no. of junctions
	//		int col_Num = Between_YNum * (ROW_COUNT - 1);

			printf("the xno.=%d, yno.=%d, and interval=%0.2f, orignInterval=%0.2f\n\n",Between_XNum,Between_YNum,base_cxstep,base_xstep);



			for(int ri = 1; ri < Base_Num + 1; ri++){
				printf("\nset node_(");
				printf("%d",(wired_startIndex + ri - 1));
				printf(") [$ns_ node]\n");

//				strtmp<<"\nset node_("<<(base_startIndex + ri - 1)<<") [$ns_ node]\n";

				printf("$node_(");
				printf("%d",(wired_startIndex + ri - 1));
				printf(") set X_ %0.4f\n",base_loc[0][ri]);

				strtmp<<"$nodew_("<<(wired_startIndex + ri - 1)<<") set X_ "<<base_loc[0][ri] + wired_xdist_base<<"\n";

				printf("$node_(");
				printf("%d",(wired_startIndex + ri - 1));
				printf(") set Y_ %0.4f\n",base_loc[1][ri]);

				strtmp<<"$nodew_("<<(wired_startIndex + ri - 1)<<") set Y_ "<<base_loc[1][ri] + wired_ydist_base<<"\n";

				printf("$node_(");
				printf("%d",(wired_startIndex + ri - 1));
				printf(") set Z_ %0.4f\n",base_loc[2][ri]);

				strtmp<<"$nodew_("<<(wired_startIndex + ri - 1)<<") set Z_ "<<base_loc[2][ri]<<"\n";

				printf("$node_(");
				printf("%d",(wired_startIndex + ri - 1));
				printf(") color \"blue\"\n");

				strtmp<<"$nodew_("<<(wired_startIndex + ri - 1)<<") color \"blue\"\n";
			}


			int New_Index = 0;

			for (int ri = 1; wired_junc_only < 1 && ri < Base_rowc; ri++) { //last row NOT add junction to next row

				for (int cj = 1; cj < Base_colc; cj++) { //last column NOT add junction to next column

					for(int bi = 1; bi < Between_XNum + 1; bi++){//extend with adding junctions to next column

						New_Index++;

						printf("\nset node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") [$ns_ node]\n");

//						strtmp<<"\nset node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") [$ns_ node]\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") set X_ %0.4f\n",base_loc[0][(ri - 1) * Base_colc + cj] + bi * base_cxstep);

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") set X_ "<<base_loc[0][(ri - 1) * Base_colc + cj] + bi * base_cxstep + wired_xdist_base<<"\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") set Y_ %0.4f\n",base_loc[1][(ri - 1) * Base_colc + cj]);

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") set Y_ "<<base_loc[1][(ri - 1) * Base_colc + cj] + wired_ydist_base<<"\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") set Z_ %0.4f\n",base_loc[2][(ri - 1) * Base_colc + cj]);

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") set Z_ "<<base_loc[2][(ri - 1) * Base_colc + cj]<<"\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") color \"red\"\n");

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") color \"red\"\n";

					}

					for(int bi = 1; bi < Between_YNum + 1; bi++){//extend with adding junctions to next row

						New_Index++;

						printf("\nset node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") [$ns node]\n");

//						strtmp<<"\nset node_("<<(base_startIndex + Base_Num - 1 + New_Index)<<") [$ns_ node]\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") set X_ %0.4f\n",base_loc[0][(ri - 1) * Base_colc + cj]);

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") set X_ "<<base_loc[0][(ri - 1) * Base_colc + cj] + wired_xdist_base<<"\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") set Y_ %0.4f\n",base_loc[1][(ri - 1) * Base_colc + cj] + bi * base_cystep);

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") set Y_ "<<base_loc[1][(ri - 1) * Base_colc + cj] + bi * base_cystep + wired_ydist_base<<"\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") set Z_ %0.4f\n",base_loc[2][(ri - 1) * Base_colc + cj]);

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") set Z_ "<<base_loc[2][(ri - 1) * Base_colc + cj]<<"\n";

						printf("$node_(");
						printf("%d",(wired_startIndex + Base_Num - 1 + New_Index));
						printf(") color \"red\"\n");

						strtmp<<"$nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") color \"red\"\n";

					}

				}

	/*			//right-orient node
				printf("\nset node_(2");
				TwoIntToChars(((ri - 1) / COL_COUNT + 1));
				TwoIntToChars(((ri - 1) % COL_COUNT + 1));*/

			}

			string contf = strtmp.str();
			wiredNodeTCL(contf);
//			printf("\n\nthe amout of wired is %d", New_Index);


}

void
ScenGene::file_pattcl(){
	int wired_startIndex = vehibases_num;//NO. is start from the id of the vehicular
	int bases_startIndex = route_vehi_num;

	std::ostringstream strtmp;
	strtmp<<"\n";

//	if(base_xstep < node_radius * 2 + 10.0 || base_ystep < node_radius * 2 + 10.0) return;


			int Base_rowc = node_rowc;
			int Base_colc = node_colc;
			int Base_Num = Base_rowc*Base_colc;

			int Between_XNum = floor((base_xstep - node_radius * 2) / node_radius / 2);

			int Between_YNum = floor((base_ystep - node_radius * 2) / node_radius / 2);

			double base_cxstep = 0.0;
			double base_cystep = 0.0;

			if(Between_XNum > 0)base_cxstep = (base_xstep) / (Between_XNum + 1); //since we want the edges, which is more than the no. of junctions
			if(Between_YNum > 0)base_cystep = (base_ystep) / (Between_YNum + 1);

	//		int row_Num = Between_XNum * (COL_COUNT - 1); //since we want the edges, which is less than the no. of junctions
	//		int col_Num = Between_YNum * (ROW_COUNT - 1);

			printf("the xno.=%d, yno.=%d, and interval=%0.2f, orignInterval=%0.2f\n\n",Between_XNum,Between_YNum,base_cxstep,base_xstep);



			for(int ri = 1; ri < Base_Num + 1; ri++){


				if(wired_junc_only == 1){
					if(ri % Base_colc != 0){
						strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + ri - 1)<<") $nodew_("<<(wired_startIndex + ri)<<") 5Mb 2ms DropTail\n";
					}
					if((ri-1) / Base_colc < Base_rowc -1){
						strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + ri - 1)<<") $nodew_("<<(wired_startIndex + ri - 1 + Base_colc)<<") 5Mb 2ms DropTail\n";
					}
					strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + ri - 1)<<") $node_("<<(bases_startIndex + ri - 1)<<") 5Mb 2ms DropTail\n";

				}else{

				}

			}


			int New_Index = 0;

			for (int ri = 1; wired_junc_only < 1 && ri < Base_rowc; ri++) { //last row NOT add junction to next row

				for (int cj = 1; cj < Base_colc; cj++) { //last column NOT add junction to next column

					for(int bi = 1; bi < Between_XNum + 1; bi++){//extend with adding junctions to next column

						New_Index++;

						if(bi == 1){
							strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") $nodew_("<<(bases_startIndex + (ri-1)*Base_rowc + cj - 1)<<") 5Mb 2ms DropTail\n";
						}else if(bi == Between_XNum){
							strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") $nodew_("<<(bases_startIndex + (ri-1)*Base_rowc + cj)<<") 5Mb 2ms DropTail\n";
						}else{
							strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index + 1)<<") 5Mb 2ms DropTail\n";
						}

					}

					for(int bi = 1; bi < Between_YNum + 1; bi++){//extend with adding junctions to next row

						New_Index++;

						if(bi == 1){
							strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") $nodew_("<<(bases_startIndex + (ri-1)*Base_rowc + cj - 1)<<") 5Mb 2ms DropTail\n";
						}else if(bi == Between_YNum){
							strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") $nodew_("<<(bases_startIndex + (ri)*Base_rowc + cj - 1)<<") 5Mb 2ms DropTail\n";
						}else{
							strtmp<<"$ns_ duplex-link $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index)<<") $nodew_("<<(wired_startIndex + Base_Num - 1 + New_Index + 1)<<") 5Mb 2ms DropTail\n";
						}

					}

				}

	/*			//right-orient node
				printf("\nset node_(2");
				TwoIntToChars(((ri - 1) / COL_COUNT + 1));
				TwoIntToChars(((ri - 1) % COL_COUNT + 1));*/

			}

			string contf = strtmp.str();
			patternTCL(contf);
//			printf("\n\nthe amout of wired is %d", New_Index);


}

void
ScenGene::parseJunc(char *junc, int from[]){
	//	printf("%s",junc);
		for(int i = 5; i < 12;i++)
			switch(junc[i]){
			case '0':
							from[(i-5) % 6] = 0;break;
			case '1':
							from[(i-5) % 6] = 1;break;
			case '2':
							from[(i-5) % 6] = 2;break;
			case '3':
							from[(i-5) % 6] = 3;break;
			case '4':
							from[(i-5) % 6] = 4;break;
			case '5':
							from[(i-5) % 6] = 5;break;
			case '6':
							from[(i-5) % 6] = 6;break;
			case '7':
							from[(i-5) % 6] = 7;break;
			case '8':
							from[(i-5) % 6] = 8;break;
			case '9':
							from[(i-5) % 6] = 9;break;
			default:
				break;

			}
	//	printf("%d%d%d,%d%d%d",from[0],from[1],from[2],from[3],from[4],from[5]);
}

bool
ScenGene::check_valid(int row, int col, int dirt) {

	/*    bool tmp = true;
		printf("\n ri=%d, ci=%d", row, col);


		//line boundary
		if(row == 1 && dirt == 0)			tmp = false;
		if(row == ROW_COUNT && dirt == 2) 	tmp = false;
		if(col == 1 && dirt == 3) 			tmp = false;
		if(col == COL_COUNT && dirt == 1) 	tmp = false;

		char right[2] = {'N','Y'};

		switch(dirt){
		case 0:
			printf("next junction is ri=%d, ci=%d valid=%c",(row-1),  col	 ,right[tmp]);break;
		case 1:
			printf("next junction is ri=%d, ci=%d valid=%c", row   , (col+1) ,right[tmp]);break;
		case 2:
			printf("next junction is ri=%d, ci=%d valid=%c",(row+1), col	 ,right[tmp]);break;
		case 3:
			printf("next junction is ri=%d, ci=%d valid=%c", row   , (col-1) ,right[tmp]);break;
		default:
			printf("ERROR!!!!!!!!!"); break;
		}


*/


		//line boundary
		if(row == 1 && dirt == 0) 			return false;
		if(row == node_rowc && dirt == 2) 	return false;
		if(col == 1 && dirt == 3) 			return false;
		if(col == node_colc && dirt == 1) 	return false;

		return true;
}

void
ScenGene::generatePoisson(int d[]){
	srand((unsigned)time(NULL));
	printf("\n==============================\n\tPoisson delay values\n");
	for(int i = 1; i < route_vehi_num + 1; i++){
		d[i] = poissonNextElement();
	}
//	geneDelay(d);

	for(int i = route_vehi_num; i > 1; i--){
		printf("\n old=%d",d[i]);
		for(int j = i - 1; j > 0; j--){
			d[i] += d[j];
		}
		printf("\t new=%d",d[i]);
	}
	printf("\n\t===     ===   finish!!!");
}

int
ScenGene::poissonNextElement(){
	float 	pd_rand = 0.1;
	int		pd_k = 0;
	float   pd_p = 0.010;
	float   pd_sump = FLT_MIN;


	pd_rand = (rand() %17) * 1.0 / 17;
	pd_k = 0;
	pd_p = 1.0 / exp(lambda);

	while(pd_rand > pd_sump){
		pd_k = pd_k + 1;
		pd_p = pd_p * lambda / pd_k;
		pd_sump = pd_sump + pd_p;
	}

	return pd_k;
//	printf("\n%d\t r=%0.6f, k =%d, p=%0.6f, sum=%0.6f", pd_k,pd_rand,pd_k,pd_p,pd_sump);
}

void
ScenGene::edge_id(int ir, int jc, int in, int jn){
	int ri = ir;
	int cj = jc;
	int ni = in;
	int nj = jn;


	printf("\n<edge id=\"1");
	if (ri > 999) {
		printf(" rowno_cc ERROR to string");
	}else if (ri > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[ri / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[ri / 100],digits[(ri % 100) / 10],digits[ri % 10]);
	}else if (ri > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[(ri % 100) / 10],digits[ri % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[0],digits[ri % 10]);

	}

	if (cj > 999) {
		printf(" colno_cc ERROR to string");
	}else if (cj > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[cj / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[cj / 100],digits[(cj % 100) / 10],digits[cj % 10]);
	}else if (cj > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[(cj % 100) / 10],digits[cj % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[0],digits[cj % 10]);

	}

	if (ni > 999) {
		printf(" rowno_next ERROR to string");
	}else if (ni > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[ri / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[ni / 100],digits[(ni % 100) / 10],digits[ni % 10]);
	}else if (ni > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[(ni % 100) / 10],digits[ni % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[0],digits[ni % 10]);

	}

	if (nj > 999) {
		printf(" colno_next ERROR to string");
	}else if (nj > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[cj / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[nj / 100],digits[(nj % 100) / 10],digits[nj % 10]);
	}else if (nj > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[(nj % 100) / 10],digits[nj % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[0],digits[nj % 10]);

	}
}

void
ScenGene::edge_ft(int ir, int jc, int in, int jn){
	int ri = ir;
	int cj = jc;
	int ni = in;
	int nj = jn;
	string   prefix = node_prefix;

	printf("\" from=\"%s", prefix.c_str());
	if (ri > 999) {
		printf(" rowno_cc ERROR to string");
	}else if (ri > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[ri / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[ri / 100],digits[(ri % 100) / 10],digits[ri % 10]);
	}else if (ri > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[(ri % 100) / 10],digits[ri % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[0],digits[ri % 10]);

	}

	if (cj > 999) {
		printf(" colno_cc ERROR to string");
	}else if (cj > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[cj / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[cj / 100],digits[(cj % 100) / 10],digits[cj % 10]);
	}else if (cj > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[(cj % 100) / 10],digits[cj % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[0],digits[cj % 10]);

	}



	printf("\" to=\"%s", prefix.c_str());
	if (ni > 999) {
		printf(" rowno_next ERROR to string");
	}else if (ni > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[ri / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[ni / 100],digits[(ni % 100) / 10],digits[ni % 10]);
	}else if (ni > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[(ri % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[(ni % 100) / 10],digits[ni % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 1] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 2] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 3] = digits[ri % 10];
		printf("%c%c%c",digits[0],digits[0],digits[ni % 10]);

	}

	if (nj > 999) {
		printf(" colno_next ERROR to string");
	}else if (nj > 99) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[cj / 100];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[nj / 100],digits[(nj % 100) / 10],digits[nj % 10]);
	}else if (nj > 9) {
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[(cj % 100) / 10];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[(nj % 100) / 10],digits[nj % 10]);
	}else{
//				tmp_Result[(ri * colC + cj) * idcharLen + 4] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 5] = digits[0];
//				tmp_Result[(ri * colC + cj) * idcharLen + 6] = digits[cj % 10];
		printf("%c%c%c",digits[0],digits[0],digits[nj % 10]);

	}

}

void
ScenGene::edge_lanes(int w){
	if (w != 0)
		printf("\" numlanes=\"%d", w);
}

void
ScenGene::edge_finishLine(){
	printf("\" >\n</edge>");
}

void
ScenGene::edge_finishFile(){
	printf("</edges>");
}


bool
ScenGene::writeXML(string fname, char openmode, string cont){
	char* foutput_name = new char[fname.size() + 1];
	strcpy(foutput_name,fname.c_str());

/*
	ofstream paraFile(foutput_name,ios::app);//append into the tail

	cout<<" writeXML append  ....."<<endl;
	paraFile<<cont.c_str()<<endl;
	paraFile.flush();
	paraFile.close();

*/


	if(openmode == '1'){
		ofstream paraFile(foutput_name,ios::app);//append into the tail
		cout<<" writeXML append  ....."<<endl;
		paraFile<<cont.c_str()<<endl;
		paraFile.flush();
		paraFile.close();
		return true;
	}else if(openmode == '0'){
		ofstream paraFile(foutput_name,ios::out);//clear & write
		cout<<" writeXML out  ...."<<endl;
		paraFile<<cont.c_str()<<endl;
		paraFile.flush();
		paraFile.close();
		return true;
	}else{
		cout<<" writeXML openmode  ERROR"<<endl;
		return false;
	}
}

void
ScenGene::nodeXML(string cont){
	writeXML(nodeFile,'0',cont);
}

void
ScenGene::edgeXML(string cont){
	writeXML(edgeFile,'0',cont);
}

void
ScenGene::routeXML(string cont){
	writeXML(routeFile,'0',cont);
}

void
ScenGene::baseStationTCL(string cont){
	writeXML(baseFile,'0',cont);
}

void
ScenGene::wiredNodeTCL(string cont){
	writeXML(wiredFile,'0',cont);
}

void
ScenGene::patternTCL(string cont){
	writeXML(patternFile,'0',cont);
}

/*
bool
ScenGene::writeXML(string fname, char openmode, ofstream* paraFile){

	char* foutput_name;
	strcpy(foutput_name,fname.c_str());
	if(openmode == '1'){
		paraFile->open(foutput_name,ios_base::app);//append into the tail

		if(paraFile->fail()){
			cout<<"\""<<fname<<"  A+ OPEN   failed"<<endl;
			return false;
		}else{
			cout<<"\""<<fname<<"  A+ OPEN   success"<<endl;
			return true;
		}
	}else if(openmode == '0'){
		paraFile->open(foutput_name,ios_base::out);//clear & write only
		if(paraFile->fail()){
			cout<<"\""<<fname<<"  W OPEN   failed"<<endl;
			return false;
		}else{
			cout<<"\""<<fname<<"  W OPEN   success"<<endl;
			return true;
		}
	}
}
*/
