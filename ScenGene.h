/*
 * ScenGene.h
 *
 *  Created on: Feb 8, 2018
 *      Author: js
 */

#ifndef SCENGENE_H_
#define SCENGENE_H_

#include "PathGene.h"
#include <fstream>
#include <string>
#include <iostream>

using namespace std;



class ScenGene{
private:


//simulator--------------------scen & ..
	double		scen_xlength;
	double 		scen_ylength;
	string		simu_prefix;
	string		file_prefix;
//node---------------junctions &  base stations
	string		node_prefix;
	int			node_rowc;
	int			node_colc;
	double		node_radius;
	double		node_xstart;
	double 		node_ystart;
	int			node_axis_type_bits;
	int			node_junc_type;
	int			node_base_station_start;
	double		node_junc_width;
//edge----------------------streets & lanes &  links
	string		edge_prefix;
	int			edge_lane_num;
	int			edge_bidirection_bits;
	int			edge_connect_bits;
	double		edge_lane_width;
	double		edge_speed_limit;
//route-----------------types of vehicular & trips & vehicle path
	string		route_vtypes_prefix;
	int			route_vtypes_num;
	double		route_vtypes_accel[2];
	double		route_vtypes_decel[2];
	double		route_vtypes_length[2];
	double		route_vtypes_maxspeed;
	double		route_vtypes_width[2];
	double		route_vtypes_sigma[2];
	double		route_vtypes_minGap;


	string		route_trip_prefix;
	int			route_trip_start;
	int			route_trip_num;
	int 		route_trip_divide_num;

	int			route_edge_range_base;
	int			route_edge_range_max;
	string		route_edge_start;
	int			route_edge_backward_cycle_bits;


	int			route_vehi_prefix;
	int			route_vehi_num;
	int			route_vehi_types;// derived from route_vtypes_num
	int			route_vehi_routes;// derived from route_trip_num
	int			route_vehi_depart;// poisson distribution
	int			route_vehi_color_bits;
	string		route_vehi_departLane;
	string		route_vehi_departSpeed;

//Poisson distribution--------------------lambda
	int			lambda;



//flow --------------------
	string		flow_pre;
	int			flow_end;
	double		vehi_emit_p;
	bool		flow_flag;
	int			flow_num;

	int			veh_kinds;
	double		bus_rate;
	double		scar_rate;
	double		texi_rate;
	double		other_rate;

//I/O-----------------files specification
	string		confFile;

	string		fileName;
	string		nodeFile;
	string		edgeFile;
	string		routeFile;
	string		settingFile;
	string		cfgFile;
	string  	baseFile;
	string  	wiredFile;
	string		patternFile;
	string		scenFile;
	string		lengFile;

//parse----------------------para & value
	string		current_line;
	string		para_name;
	string		para_value_one;
	string		para_value_two;


//base-stations-----------------------
	//the following are for base station generation
	double** base_loc;
	double base_xstep;
	double base_ystep;
	int		base_startIndex;

//wired-nodes--------------------------
	int  wired_junc_only;
	double wired_xdist_base;
	double wired_ydist_base;
	int  wired_boundary_valid_bits;
	double delay_rate;
	bool wired_delay_show_flag;


//inner variables----------------------------
	int vehibases_num;
	int base_num;
	int x_add_nodes;
	int y_add_nodes;
	double mini_dist_rate;
	double dist_Xx_add_nodes;
	double dist_Yy_add_nodes;

	bool  rand_order;

	PathGene *spg;
	int bus_num;
	int scar_num;
	int texi_num;
	int other_num;
	int route_shortest_path_num;
	int route_loop_path_num;

//-------------experimental data, abstracted from Two examples of simulations to debug the show of nam ------------------
	double miniAxisStepA;
	double maxiAxisStepB;
	double miniDelayStepA;
	double maxiDelayStepB;

	double precise_junc_delayX;
	double precise_junc_delayY;
	double precise_base_delayX;
	double precise_base_delayY;



	double UPlengthJuncIJ[20][20];
	double RIGHTlengthJuncIJ[20][20];
	double UPangelJuncIJ[20][20];
	double RIGHTangelJuncIJ[20][20];


//	ofstream file_handle;
private:

	void parseLine(int type,int index);
	void parseValue(int type,int index);
	void nodePara(int);
	void edgePara(int);
	void routePara(int);
	void filePara(int);
	void wiredPara(int);
	void flowPara(int);


	void edge_id(int,int,int,int);
	void edge_ft(int,int,int,int);
	void edge_lanes(int l=0);
	void edge_finishLine();
	void edge_finishFile();

	void printPara();

	void IntToChars(int);
	void TwoIntToChars(int);

	void IntToChars(int,char*);
	void TwoIntToChars(int,char*);

	bool writeXML(string,char, string);

	void nodeXML(string);
	void edgeXML(string);
	void routeXML(string);
	void baseStationTCL(string);
	void wiredNodeTCL(string);
	void patternTCL(string);
	void scenarioXML(string);
	void roadLengthXML(string);

	bool check_valid(int,int,int);
	void parseJunc(char*, int[]);

	void generatePoisson(int[]);
	int  poissonNextElement();

	void InitshortestPath();
	void shortestPath(int,int,int,int,int[]);


public:
	ScenGene();
	~ScenGene();
	void readconf();
	void file_nodxml();
	void file_edgxml();
	void file_rouxml();
	void file_bastcl();
	void file_wirtcl();
	void file_pattcl();
	void file_scexml();
	void file_lenxml();

	void file_NODExml();
	void file_ROUTExml();


};


#endif /* SCENGENE_H_ */
