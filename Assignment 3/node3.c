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
} dt3;

/* students to write the following two routines, and maybe some others */

void rtinit3() 
{
  for(int i=0; i<4; i++)
  {
    for(int j=0; j<4; j++)
    {
      dt3.costs[i][j] = 999;
    }
  }
  // entry [i,j] in the distance table in node 0 is node 0's currently computed cost to node i via direct neighbor j
  dt3.costs[0][0] = 7;
  dt3.costs[2][2] = 2;
  dt3.costs[3][3] = 0;
  struct rtpkt sndpkt;
  sndpkt.sourceid = 3;
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt3.costs[i][j]<min_cost)
      {
        min_cost = dt3.costs[i][j];
      }
    }
    sndpkt.mincost[i] = min_cost;
  }
  sndpkt.destid = 0;
  tolayer2(sndpkt);
  sndpkt.destid = 2;
  tolayer2(sndpkt);
}


void rtupdate3(rcvdpkt)
  struct rtpkt *rcvdpkt;
  
{
  int flag = 0;
  if(rcvdpkt->destid!=3)
    exit(0);
  int before[4];
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt3.costs[i][j]<min_cost)
      {
        min_cost = dt3.costs[i][j];
      }
    }
    before[i] = min_cost;
  }

  for(int i=0; i<4; i++)
  {
    if(dt3.costs[i][rcvdpkt->sourceid] != rcvdpkt->mincost[i] + dt3.costs[rcvdpkt->sourceid][rcvdpkt->sourceid])
    {
      dt3.costs[i][rcvdpkt->sourceid] = rcvdpkt->mincost[i] + dt3.costs[rcvdpkt->sourceid][rcvdpkt->sourceid];
    }
  }
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt3.costs[i][j]<min_cost)
      {
        min_cost = dt3.costs[i][j];
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
  sndpkt.sourceid = 3;
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt3.costs[i][j]<min_cost)
      {
        min_cost = dt3.costs[i][j];
      }
    }
    sndpkt.mincost[i] = min_cost;
  }
  sndpkt.destid = 0;
  tolayer2(sndpkt);
  sndpkt.destid = 2;
  tolayer2(sndpkt);
}


void printdt3(dtptr)
  struct distance_table *dtptr;
  
{
  printf("             via     \n");
  printf("   D3 |    0     2 \n");
  printf("  ----|-----------\n");
  printf("     0|  %3d   %3d\n",dtptr->costs[0][0], dtptr->costs[0][2]);
  printf("dest 1|  %3d   %3d\n",dtptr->costs[1][0], dtptr->costs[1][2]);
  printf("     2|  %3d   %3d\n",dtptr->costs[2][0], dtptr->costs[2][2]);

}







