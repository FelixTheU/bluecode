#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#define SENDER_SOCK "/tmp/udsmbench_sender"
#define RECEVER_SOCK "/tmp/udsmbench_recv_%d"

void printUsage(char *pProgrmName)
{
  printf("%s [-s or -r] [-n recverNum] [-m modId]\n", pProgrmName);
}

void asSender(int modNum)
{
  int lSts = 0;
  int lRet = 0;
  int sdrFd = socket(AF_UNIX, SOCK_DGRAM, 0);

  lSts = fcntl(sdrFd, F_GETFL, 0);
 lSts =   fcntl(sdrFd, F_SETFL, lSts | O_NONBLOCK);
 
  
  struct sockaddr_un sdrAddr;
  sdrAddr.sun_family = AF_UNIX;
  strcpy(sdrAddr.sun_path, SENDER_SOCK);

  unlink(SENDER_SOCK);
  lSts = bind(sdrFd, (struct sockaddr *)&sdrAddr, sizeof (sdrAddr));

  /* prepare mods's addr */
  struct sockaddr_un *pModsAddr = (struct sockaddr_un *)malloc(sizeof (struct sockaddr_un) * modNum);
  int *pRemainMap = (int *)malloc(sizeof(int) * modNum);
  int i = 0;
  char modSockPathBuf[50] = { 0 };
  
  int sizeK = 0, sendTimes = 0;
  while (1)
    {
      printf("while.\n");
      char inBuf[100] = { 0 };
      read(0, inBuf, sizeof (inBuf));
      sscanf(inBuf, "sendto %d %d\n", &sizeK, &sendTimes);
      printf("sizeK:%d, sendTimes:%d.\n", sizeK, sendTimes);
  
  for (i = 0; i < modNum; i++)
    {
      pModsAddr[i].sun_family = AF_UNIX;
      sprintf(modSockPathBuf, RECEVER_SOCK, i);
      strcpy(pModsAddr[i].sun_path, modSockPathBuf);
      pRemainMap[i] = sendTimes;
    }

  int sendSize = sizeK * 1024;
  char *pSendBuf = (char *)malloc(sendSize);
  bzero(pSendBuf, sendSize);

  int bRemain = 1;
  printf("开始发送, at %lu\n", time(NULL));
  while (bRemain)
    {
      
  for (i = 0; i < modNum; i++)
    {
      while (pRemainMap[i] > 0)
	{
	  printf("before sendto.\n");
	  lSts = sendto(sdrFd, pSendBuf, sendSize, 0,  (struct sockaddr *)&pModsAddr[i], sizeof (struct sockaddr_un));

	  if (lSts < 0 && EAGAIN == errno)
	    {
	      printf("EAGAIN.\n");
	      break;
	    }
	  if (lSts < 0)
	    {
	      printf("发送出错.\n");
	      return ;
	    }
	  printf("after sendto.\n");
	  printf("remain: %d.\n", pRemainMap[i]);
	  if (--pRemainMap[i] == 0)
	    {			/* 发送结束消息 */
	      pSendBuf[0] = '#';
	      lSts = sendto(sdrFd, pSendBuf, sendSize, 0, (struct sockaddr *)&pModsAddr[i], sizeof (struct sockaddr_un));
	      if (lSts > 0 )
		{
		  printf("结束消息发送成功.\n");
		}
	      pSendBuf[0] = 0;
	    }
	}

      int j = 0;
      for (j = 0; j < modNum; j++)
	{
	  if (pRemainMap[i] != 0)
	    {
	      goto goon;
	    }
	}
      
      bRemain = 0;
      printf("消息发送完成 at %lu\n", time(NULL));
      break;
    goon:
      continue;
    }
    }
    }
  
}

void asRecever(int modNum)
{
  char recvSockBuf[30] = { 0 };
  sprintf(recvSockBuf, RECEVER_SOCK, modNum);
  
  int recvFd = socket(AF_UNIX, SOCK_DGRAM, 0);
  struct sockaddr_un recvAddr;
  recvAddr.sun_family = AF_UNIX;
  strcpy(recvAddr.sun_path, recvSockBuf);

  unlink(recvSockBuf);
  bind(recvFd, (struct sockaddr *)&recvAddr, sizeof (recvAddr));

  struct sockaddr_un sdrAddr;
  socklen_t addrLen = sizeof (sdrAddr);
  char recvBuf[1024 * 100] = { 0 };
  int lSts = 0;
  unsigned long ulRecvCnt = 0;
  
  while (1)
    {
      lSts = recvfrom(recvFd, recvBuf, sizeof (recvBuf), 0, (struct sockaddr *)&sdrAddr, &addrLen);
      if (lSts < 0 )
	{
	  return ;
	}
      printf("收到消息.\n");

      ulRecvCnt++;
      if (*recvBuf == '#')
	{
	  printf("消息接收完毕，共接收 %d, at %lu.\n", ulRecvCnt, time(NULL));
	}
      
    }
    
}
int main(int argc, char *argv[])
{
  int lSts = 0;
  int lRet = 0;
  int bAsSender = 0;
  int bAsRecever = 0;
  int toModNum = 0;
  int modId = 0;
  char *optStr = "srn:m:";
  int opt = 0;
  if (argc < 2)
    {
      printUsage(argv[0]);
      return 0;
    }

  while ((opt = getopt(argc, argv, optStr)) != -1)
    {
      switch (opt)
	{
	case 's':
	  bAsSender = 1;
	  break;
	case 'r':
	  bAsRecever = 1;
	  break;
	case 'n':
	  toModNum = atoi(optarg);
	  break;
	case 'm':
	  modId = atoi(optarg);
	  break;
	}
    }

  if (bAsSender)
    {
      asSender(toModNum);
    }
  else if(bAsRecever)
    {
      asRecever(modId);
    }

  return 0;
}
