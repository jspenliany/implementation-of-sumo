/*
 * PathGene.cc
 *
 *  Created on: May 10, 2018
 *      Author: js
 */
#include "PathGene.h"

PathGene::PathGene(){
	node_num = 1;
	link_num = 0;
}

PathGene::PathGene(int nodeNum){
	node_num = nodeNum;
	link_num = 0;
}

PathGene::PathGene(int nnum, int lnum){
	node_num = nnum;
	link_num = lnum;
}

void
PathGene::init(){
    for(int i=0; i<=node_num;i++){
    	dist[i] = 299999999;
    	vis[i]  = true;
    	pre[i] = i;
        for(int j=0;j<=node_num;j++) link[i][j] = 299999999;
    }
}

void
PathGene::init_length(int src, int dst, double weight){
    if(weight<link[src][dst]){
    	link[src][dst] = (int)weight;
    	link[dst][src] = (int)weight;
    }
}

void
PathGene::Dijkstra(int sid){
	dist[sid] = 0;
	vis[sid]  = false;

	int MAXINT = 299999999;

/*
	for(int k=0; k<node_num; k++){
		printf("\n");
		for(int i=0; i<node_num; ++i){
			printf("%d %d %d\t",k,i,link[k][i]);
		}
	}
*/

	for(int k=0; k<node_num; k++){
		int nextElement = -1;
		int preElement = -1;

		int finalPrevious = -1;
		int finalSelect = -1;
		int finalLength = MAXINT/2;

		int nextLength = MAXINT/2;
		for(int i=0; i<node_num; ++i){
			if(!vis[i]){
				for(int j=0; j<node_num; j++){
					if(vis[j]){
						if(dist[i] + link[i][j] < nextLength){
							nextElement = j;
							preElement = i;
							nextLength = dist[i] + link[i][j];
						}
					}
				}

				if(nextLength < finalLength){
					finalPrevious = preElement;
					finalSelect = nextElement;
					finalLength = nextLength;
				}
			}
		}

		pre[finalSelect] = finalPrevious;
		dist[finalSelect] = dist[finalPrevious] + link[finalPrevious][finalSelect];
		vis[finalSelect] = false;
		printf("\ni=%d, j=%d dist=%d length=%d",finalPrevious,finalSelect,link[finalPrevious][finalSelect],dist[finalSelect]);
	}
}

void
PathGene::dfs(int sid){
    if(pre[sid]==sid){
        printf("%d", sid);
        return;
    }
    dfs(pre[sid]);
    printf(" -> %d",sid);
    return;
}

int
PathGene::getPrevious(int sid){
	return pre[sid];
}
