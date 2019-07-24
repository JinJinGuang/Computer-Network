#include <stdio.h>
#include <stdlib.h>

extern struct rtpkt {
  int sourceid;       /* id of sending router sending this pkt */
  int destid;         /* id of router to which pkt being sent 
                         (must be an immediate neighbor) */
  int mincost[4];    /* min cost to node 0 ... 3 */
  };

extern int TRACE;
extern int YES;
extern int NO;

extern void tolayer2(struct rtpkt packet);


struct distance_table 
{
  int costs[4][4];
} dt2;


/* students to write the following two routines, and maybe some others */

void rtinit2() 
{
  for(int i=0; i<4; i++)
  {
    for(int j=0; j<4; j++)
    {
      dt2.costs[i][j] = 999;
    }
  }
  // entry [i,j] in the distance table in node 0 is node 0's currently computed cost to node i via direct neighbor j
  dt2.costs[0][0] = 3;
  dt2.costs[2][2] = 0;
  dt2.costs[1][1] = 1;
  dt2.costs[3][3] = 2;
  struct rtpkt sndpkt;
  sndpkt.sourceid = 2;
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt2.costs[i][j]<min_cost)
      {
        min_cost = dt2.costs[i][j];
      }
    }
    sndpkt.mincost[i] = min_cost;
  }
  sndpkt.destid = 0;
  tolayer2(sndpkt);
  sndpkt.destid = 1;
  tolayer2(sndpkt);
  sndpkt.destid = 3;
  tolayer2(sndpkt);
}


void rtupdate2(rcvdpkt)
  struct rtpkt *rcvdpkt;
  
{
  int flag = 0;
  if(rcvdpkt->destid!=2)
    exit(0);
  int before[4];
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt2.costs[i][j]<min_cost)
      {
        min_cost = dt2.costs[i][j];
      }
    }
    before[i] = min_cost;
  }

  for(int i=0; i<4; i++)
  {
    if(dt2.costs[i][rcvdpkt->sourceid] != rcvdpkt->mincost[i] + dt2.costs[rcvdpkt->sourceid][rcvdpkt->sourceid])
    {
      dt2.costs[i][rcvdpkt->sourceid] = rcvdpkt->mincost[i] + dt2.costs[rcvdpkt->sourceid][rcvdpkt->sourceid];
    }
  }
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt2.costs[i][j]<min_cost)
      {
        min_cost = dt2.costs[i][j];
      }
    }
    if(before[i]!=min_cost)
    {
      before[i] = min_cost;
      flag = 1;
    }
  }
  if(flag==0)
    return;
  struct rtpkt sndpkt;
  sndpkt.sourceid = 2;
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt2.costs[i][j]<min_cost)
      {
        min_cost = dt2.costs[i][j];
      }
    }
    sndpkt.mincost[i] = min_cost;
  }
  sndpkt.destid = 0;
  tolayer2(sndpkt);
  sndpkt.destid = 1;
  tolayer2(sndpkt);
  sndpkt.destid = 3;
  tolayer2(sndpkt);
}


void printdt2(dtptr)
  struct distance_table *dtptr;
  
{
  printf("                via     \n");
  printf("   D2 |    0     1    3 \n");
  printf("  ----|-----------------\n");
  printf("     0|  %3d   %3d   %3d\n",dtptr->costs[0][0],
	 dtptr->costs[0][1],dtptr->costs[0][3]);
  printf("dest 1|  %3d   %3d   %3d\n",dtptr->costs[1][0],
	 dtptr->costs[1][1],dtptr->costs[1][3]);
  printf("     3|  %3d   %3d   %3d\n",dtptr->costs[3][0],
	 dtptr->costs[3][1],dtptr->costs[3][3]);
}







