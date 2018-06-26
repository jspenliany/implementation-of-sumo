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
    	pre[i] = -100;
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
	pre[sid] = sid;

	int MAXINT = 299999999;


	for(int pD_i=0; pD_i < node_num; pD_i++){
		cout<<"_|";
		//find the minimum
		int tmp_MAX = MAXINT;
		int tmp_index = -1;
		for(int nD_i=0; nD_i < node_num; nD_i++){
			if(vis[nD_i]){
				if(dist[nD_i] < tmp_MAX){
					tmp_MAX = dist[nD_i];
					tmp_index = nD_i;
				}
			}
		}
		vis[tmp_index] = false;
		//update the remains
		for(int nD_i=0; nD_i < node_num; nD_i++){
			if(vis[nD_i]){
				if(dist[nD_i] > dist[tmp_index] + link[tmp_index][nD_i]){
					dist[nD_i] = dist[tmp_index] + link[tmp_index][nD_i];
					pre[nD_i] = tmp_index;
				}
			}
		}

//		printf("\ni=%d, j=%d dist=%d length=%d",pre[tmp_index],tmp_index,link[pre[tmp_index]][tmp_index],dist[tmp_index]);
	}
	cout<<"\n";
/*
	vis[sid]  = false;
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
	}*/
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
