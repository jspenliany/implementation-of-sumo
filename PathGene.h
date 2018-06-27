/*
 * PathGene.h
 *
 *  Created on: May 10, 2018
 *      Author: js
 */

#ifndef PATHGENE_H_
#define PATHGENE_H_

#include <iostream>
#include <cstdio>

using namespace std;


class PathGene{
public:
	PathGene();
	PathGene(int);
	PathGene(int,int);

	void init();
	void init_length(int,int,double);
	void Dijkstra(int);
	void dfs(int);

	int	 getPrevious(int);

private:
	int 	node_num;
	int 	link_num;
	int		pre[1000];
	bool	vis[1000];
	int		dist[1000];
	int		link[1000][1000];
};




#endif /* PATHGENE_H_ */
