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
} dt0;


/* students to write the following two routines, and maybe some others */

void send_packet0()
{
  struct rtpkt sndpkt;
  sndpkt.sourceid = 0;
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt0.costs[i][j]<min_cost)
      {
        min_cost = dt0.costs[i][j];
      }
    }
    sndpkt.mincost[i] = min_cost;
  }
  sndpkt.destid = 1;
  tolayer2(sndpkt);
  sndpkt.destid = 2;
  tolayer2(sndpkt);
  sndpkt.destid = 3;
  tolayer2(sndpkt);
}

void rtinit0() 
{
  for(int i=0; i<4; i++)
  {
    for(int j=0; j<4; j++)
    {
      dt0.costs[i][j] = 999;
    }
  }
  // entry [i,j] in the distance table in node 0 is node 0's currently computed cost to node i via direct neighbor j
  dt0.costs[0][0] = 0;
  dt0.costs[1][1] = 1;
  dt0.costs[2][2] = 3;
  dt0.costs[3][3] = 7;
  send_packet0();
}


void rtupdate0(struct rtpkt *rcvdpkt)
{
  int flag = 0;
  if(rcvdpkt->destid!=0)
    exit(0);

  int before[4];
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt0.costs[i][j]<min_cost)
      {
        min_cost = dt0.costs[i][j];
      }
    }
    before[i] = min_cost;
  }

  for(int i=0; i<4; i++)
  {
    if(dt0.costs[i][rcvdpkt->sourceid] != rcvdpkt->mincost[i] + dt0.costs[rcvdpkt->sourceid][rcvdpkt->sourceid])
    {
      dt0.costs[i][rcvdpkt->sourceid] = rcvdpkt->mincost[i] + dt0.costs[rcvdpkt->sourceid][rcvdpkt->sourceid];
    }
  }
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt0.costs[i][j]<min_cost)
      {
        min_cost = dt0.costs[i][j];
      }
    }
    if(before[i]!=min_cost)
    {
      before[i] = min_cost;
      flag = 1;
    }
  }

  
  if(flag!=0)
    send_packet0();
}


void printdt0(struct distance_table *dtptr)
{
  printf("                via     \n");
  printf("   D0 |    1     2    3 \n");
  printf("  ----|-----------------\n");
  printf("     1|  %3d   %3d   %3d\n",dtptr->costs[1][1],
	 dtptr->costs[1][2],dtptr->costs[1][3]);
  printf("dest 2|  %3d   %3d   %3d\n",dtptr->costs[2][1],
	 dtptr->costs[2][2],dtptr->costs[2][3]);
  printf("     3|  %3d   %3d   %3d\n",dtptr->costs[3][1],
	 dtptr->costs[3][2],dtptr->costs[3][3]);
}


void linkhandler0(int linkid, int newcost)   

/* called when cost from 0 to linkid changes from current value to newcost*/
/* You can leave this routine empty if you're an undergrad. If you want */
/* to use this routine, you'll need to change the value of the LINKCHANGE */
/* constant definition in prog3.c from 0 to 1 */
	
{
  int before[4];
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt0.costs[i][j]<min_cost)
      {
        min_cost = dt0.costs[i][j];
      }
    }
    before[i] = min_cost;
  }

  int flag = 0;
  int pre = dt0.costs[linkid][linkid];
  for(int i=0; i<4; i++)
  {
    dt0.costs[i][linkid] = dt0.costs[i][linkid] - pre + newcost;
  }

  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt0.costs[i][j]<min_cost)
      {
        min_cost = dt0.costs[i][j];
      }
    }
    if(before[i]!=min_cost)
    {
      before[i] = min_cost;
      flag = 1;
    }
  }

  if(flag!=0)
    send_packet0();
}

