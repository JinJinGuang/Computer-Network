#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 打印消息
#define Pt 0

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   
   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0 /* change to 1 if you're doing extra credit */
                        /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg
{
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt
{
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer3(int AorB, struct pkt packet);
void tolayer5(int AorB, char datasent[20]);

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
////////////////////////////////////////////////////
// 公共实现的函数： 

/// 检验和相关：

//// Checkout Function 采用直接加和
int Checksum(struct pkt *packet){
    int sum = packet->acknum + packet->seqnum;
    for(int i = 0; i<20; i++)
    {
        sum += packet->payload[i];
    }
    return sum;
}
//// is corrupted or not
int IsCorrupt(struct pkt* packet) {
	if (Checksum(packet) == packet->checksum) 
    {
		return 0;
	} 
    else 
    {
		return 1;
	}
}

/// msg 与 pkt 相互转换
//// msg to pkt
void msg_to_pkt(struct msg *message, struct pkt *packet, int seq, int ack){
    packet->acknum = ack;
    packet->seqnum = seq;
    for(int i=0; i<20; i++)
    {
        packet->payload[i] = message->data[i];
    }
    packet->checksum = Checksum(packet);
}
//// pkt to msg
void pkt_to_msg(struct msg *message, struct pkt *packet){
    for(int i=0; i<20; i++)
    {
        message->data[i] = packet->payload[i];
    }
}

////////////////////////////////////////////////////////////
// A 端

/// 状态变量与全局变量

//// 上层发来的数据缓存（采用循环队列）
# define N 1000 // Buffer 大小
struct msg BufferA[N];
int endA; // 上层共送来了 endA 个msg，endA 有可能比N大（应该换个名字）

//// 状态变量
int baseA; // 正在传送这个 也用此当作ack序号
int wait; // bool: wait is 1, ok is 0
////// wait 用来识别是否在发送pkt状态，只在一开始启动有作用

//// 临时变量（可以省掉）
struct msg sendmsg; // 当前正在操作的msg
struct pkt sendpkt; // 当前正在操作的pkt

/// 初始化
/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void)
{
    baseA = 0;
    endA = 0;
    wait = 0;
}

/// 上层传送消息给A
/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    // step1: 存取msg，放入BufferA
    // 此处如果传入的数据太快而N太小会导致数据覆盖（方法：舍弃）
    if((endA-baseA)<N)
    {
        BufferA[endA%N] = message;
        endA++;
        if(Pt)
            printf("A get msg %d by layer5\n", endA-1);
    }
    else
    {
        if(Pt)
            printf("Buffer A overflow!\n");
    }

    // step2: 如果之前未工作，开始工作
    // 如果之前的msg都发掉了，这里启动继续发送
    if(!wait)
    {
        sendmsg = BufferA[baseA%N];
        msg_to_pkt(&sendmsg, &sendpkt, baseA, -1);
        wait = 1;
        tolayer3(0, sendpkt);
        starttimer(0, 20.0);
        if(Pt)
            printf("A snd pkt %d by layer3\n", sendpkt.seqnum);
    }
}

/// 下层收到ACK
/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    //printf("A input %d %d\n", packet.seqnum, packet.acknum);
    if((!IsCorrupt(&packet)) && (packet.acknum==(baseA + 1)))
    {
        stoptimer(0);
        baseA = baseA + 1;
        wait = 0;
        if(Pt)
            printf("A rcv ack %d by layer3\n", packet.acknum);
        //printf("base %d, end %d\n", baseA, endA);
        // 接收到可以马上继续传送
        if(baseA < endA)
        {
            sendmsg = BufferA[baseA%N];
            msg_to_pkt(&sendmsg, &sendpkt, baseA, -1);
            wait = 1;
            tolayer3(0, sendpkt);
            starttimer(0, 20.0);
            if(Pt)
                printf("A snd pkt %d to layer3\n", sendpkt.seqnum);
        }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt(void)
{
    //printf("A timeout %d %d\n",sendpkt.seqnum, sendpkt.acknum);
    tolayer3(0, sendpkt);
    if(Pt)
        printf("A snd pkt %d to layer3\n", sendpkt.seqnum);

    starttimer(0, 20.0);
}



/* Note that with simplex transfer from a-to-B, there is no B_output() */

// emmmmm不知道这是个啥
void send_ack(int AorB, int ack)
{
}

//////////////////////////////////////////////////////////////
// B端
int SeqB;
int AckB;

struct pkt ackpkt;
/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void)
{
    SeqB = -1;
    AckB = 0;
    ackpkt.acknum = AckB;
    ackpkt.seqnum = SeqB;
    ackpkt.checksum = Checksum(&ackpkt);
}
/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if((!IsCorrupt(&packet)) && (packet.seqnum==AckB))
    {
        tolayer5(1, packet.payload);
        ackpkt.acknum = (packet.seqnum + 1) ;
        ackpkt.seqnum = SeqB;
        ackpkt.checksum = Checksum(&ackpkt);
        tolayer3(1, ackpkt);
        AckB++;
        if(Pt)
            printf("B rcv pkt %d by layer3\n", packet.seqnum);
        if(Pt)
            printf("B snd ack %d to layer3\n", ackpkt.acknum);
    } 
    else if((!IsCorrupt(&packet)) && (packet.seqnum!=AckB))
    {
        tolayer3(1, ackpkt);
        if(Pt)
            printf("B rcv pkt %d by layer3\n", packet.seqnum);
        if(Pt)
            printf("B snd ack %d to layer3\n", ackpkt.acknum);
    }
    else
    {
        if(Pt)
            printf("Is corrupt!\n");
        if(Pt)
            printf("CheckSum %d and should is %d\n", packet.checksum, Checksum(&packet));
    }
    
}

/* need be completed only for extra credit */
void B_output(struct msg message)
{
    printf("  B_output: uni-directional. ignore.\n");
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");
}



/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)
THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct pkt *pktptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;   /* for my debugging */
int nsim = 0;    /* number of messages from 5 to 4 so far */
int nsimmax = 0; /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped  */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer3;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

void init(int argc, char **argv);
void generate_next_arrival(void);
void insertevent(struct event *p);

int main(int argc, char **argv)
{
    struct event *eventptr;
    struct msg msg2give;
    struct pkt pkt2give;

    int i, j;
    char c;

    init(argc, argv);
    A_init();
    B_init();

    while (1)
    {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER5)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in msg to give with string of same letter */
                j = nsim % 26;
                for (i = 0; i < 20; i++)
                    msg2give.data[i] = 97 + j;
                msg2give.data[19] = 0;
                if (TRACE > 2)
                {
                    printf("          MAINLOOP: data given to student: ");
                    for (i = 0; i < 20; i++)
                        printf("%c", msg2give.data[i]);
                    printf("\n");
                }
                nsim++;
                if (eventptr->eventity == A)
                    A_output(msg2give);
                else
                    B_output(msg2give);
            }
        }
        else if (eventptr->evtype == FROM_LAYER3)
        {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give);       /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(
        " Simulator terminated at time %f\n after sending %d msgs from layer5\n",
        time, nsim);
}

void init(int argc, char **argv) /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    if (argc != 6)
    {
        printf("usage: %s  num_sim  prob_loss  prob_corrupt  interval  debug_level\n", argv[0]);
        exit(1);
    }

    nsimmax = atoi(argv[1]);
    lossprob = atof(argv[2]);
    corruptprob = atof(argv[3]);
    lambda = atof(argv[4]);
    TRACE = atoi(argv[5]);
    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("the number of messages to simulate: %d\n", nsimmax);
    printf("packet loss probability: %f\n", lossprob);
    printf("packet corruption probability: %f\n", corruptprob);
    printf("average time between messages from sender's layer5: %f\n", lambda);
    printf("TRACE: %d\n", TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(void)
{
    double mmm = RAND_MAX;
    float x;          /* individual students may need to change mmm */
    x = rand() / mmm; /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(void)
{
    double x, log(), ceil();
    struct event *evptr;
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
                                 /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

void insertevent(struct event *p)
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist; /* q points to header of list in which p struct inserted */
    if (q == NULL)
    { /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)
        { /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)
        { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else
        { /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist(void)
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
               q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB /* A or B is trying to stop timer */)
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;        /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)
            { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else
            { /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB /* A or B is trying to stop timer */, float increment)
{
    struct event *q;
    struct event *evptr;

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER3 ***************/
void tolayer3(int AorB /* A or B is trying to stop timer */, struct pkt packet)
{
    struct pkt *mypktptr;
    struct event *evptr, *q;
    float lastime, x;
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2)
    {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
                                      /* finally, compute the arrival time of packet at the other end.
                                         medium can not reorder, so make sure packet arrives between 1 and 10
                                         time units after the latest arrival time of packets
                                         currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer5(int AorB, char datasent[20])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}
