#include <stdio.h>
#include <stdlib.h>

extern struct rtpkt {
  int sourceid;       /* id of sending router sending this pkt */
  int destid;         /* id of router to which pkt being sent 
                         (must be an immediate neighbor) */
  int mincost[4];    /* min cost to node 0 ... 3 */
  };

extern void tolayer2(struct rtpkt packet);


extern int TRACE;
extern int YES;
extern int NO;

int connectcosts1[4] = { 1,  0,  1, 999 };

struct distance_table 
{
  int costs[4][4];
} dt1;


/* students to write the following two routines, and maybe some others */

void send_packet1()
{
  struct rtpkt sndpkt;
  sndpkt.sourceid = 1;
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt1.costs[i][j]<min_cost)
      {
        min_cost = dt1.costs[i][j];
      }
    }
    sndpkt.mincost[i] = min_cost;
  }
  sndpkt.destid = 0;
  tolayer2(sndpkt);
  sndpkt.destid = 2;
  tolayer2(sndpkt);
}

void rtinit1() 
{
  for(int i=0; i<4; i++)
  {
    for(int j=0; j<4; j++)
    {
      dt1.costs[i][j] = 999;
    }
  }
  // entry [i,j] in the distance table in node 0 is node 0's currently computed cost to node i via direct neighbor j
  dt1.costs[0][0] = 1;
  dt1.costs[1][1] = 0;
  dt1.costs[2][2] = 1;
  send_packet1();
}


void rtupdate1(rcvdpkt)
  struct rtpkt *rcvdpkt;
  
{
  int flag = 0;
  if(rcvdpkt->destid!=1)
    exit(0);
  int before[4];
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt1.costs[i][j]<min_cost)
      {
        min_cost = dt1.costs[i][j];
      }
    }
    before[i] = min_cost;
  }

  for(int i=0; i<4; i++)
  {
    if(dt1.costs[i][rcvdpkt->sourceid] != rcvdpkt->mincost[i] + dt1.costs[rcvdpkt->sourceid][rcvdpkt->sourceid])
    {
      dt1.costs[i][rcvdpkt->sourceid] = rcvdpkt->mincost[i] + dt1.costs[rcvdpkt->sourceid][rcvdpkt->sourceid];
    }
  }
  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt1.costs[i][j]<min_cost)
      {
        min_cost = dt1.costs[i][j];
      }
    }
    if(before[i]!=min_cost)
    {
      before[i] = min_cost;
      flag = 1;
    }
  }
  if(flag!=0)
    send_packet1();
}


void printdt1(dtptr)
  struct distance_table *dtptr;
  
{
  printf("             via   \n");
  printf("   D1 |    0     2 \n");
  printf("  ----|-----------\n");
  printf("     0|  %3d   %3d\n",dtptr->costs[0][0], dtptr->costs[0][2]);
  printf("dest 2|  %3d   %3d\n",dtptr->costs[2][0], dtptr->costs[2][2]);
  printf("     3|  %3d   %3d\n",dtptr->costs[3][0], dtptr->costs[3][2]);

}



void linkhandler1(linkid, newcost)   
int linkid, newcost;   
/* called when cost from 1 to linkid changes from current value to newcost*/
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
      if(dt1.costs[i][j]<min_cost)
      {
        min_cost = dt1.costs[i][j];
      }
    }
    before[i] = min_cost;
  }

  int flag = 0;
  int pre = dt1.costs[linkid][linkid];
  for(int i=0; i<4; i++)
  {
    dt1.costs[i][linkid] = dt1.costs[i][linkid] - pre + newcost;
  }

  for(int i=0; i<4; i++)
  {
    int min_cost = 999;
    for(int j=0; j<4; j++)
    {
      if(dt1.costs[i][j]<min_cost)
      {
        min_cost = dt1.costs[i][j];
      }
    }
    if(before[i]!=min_cost)
    {
      before[i] = min_cost;
      flag = 1;
    }
  }

  if(flag!=0)
    send_packet1();
}


