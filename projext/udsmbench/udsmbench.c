#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

#define SENDER_SOCK "/tmp/udsmbench_sender"
#define RECVER_SOCK "/tmp/udsmbench_recv_%d"
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
  fcntl(sdrFd, F_SETFL, lSts | O_NONBLOCK);
  
  struct sockaddr_un sdrAddr;
  sdrAddr.sun_family = AF_UNIX;
  strcpy(sdrAddr.sun_path, SENDER_SOCK);

  lSts = bind(sdrFd, (struct sockaddr *)&sdrAddr, sizeof (sdrAddr));

  /* prepare mods's addr */
  struct sockaddr_un *pModsAddr = (struct sockaddr_un *)malloc(sizeof (struct sockaddr_un) * modNum);
  int *pRemainMap = (int *)malloc(sizeof(int) * modNum);
  int i = 0;
  char modSockPathBuf[50] = { 0 };
  
  int sizeK = 0, sendTimes = 0;
  scanf("sendto %d %d", &sizeK, &sendTimes);
  
  for (; i < modNum; i++)
    {
      pModsAddr[i].sun_family = AF_UNIX;
      sprintf(modSockPathBuf, RECVER_SOCK, i);
      strcpy(pModsAddr[i].sun_path, modSockPathBuf);
      pRemainMap[i] = sendTimes;
    }

  int sendSize = sizeK * 1024;
  char *pSendBuf = (char *)malloc(sendSize);
  bzero(pSendBuf, sendSize);

  printf("开始发送, at %lu\n", time);
  for (i = 0; i < modNum; i++)
    {
      while (pRemainMap[i] > 0)
	{
	  lSts = sendto(sdrFd, pSendBuf, sendSize, (struct sockaddr *)&pModsAddr[i], sizeof (struct sockaddr_un), 0);
	  if (lSts < 0 && EAGAIN == errno)
	    {
	      break;
	    }
	  if (lSts < 0)
	    {
	      return ;
	    }

	  pRemainMap[i]--;
	}

      int j = 0;
      for (j = 0; j < modNum; j++)
	{
	  if (pRemainMap[i] != 0)
	    {
	      goto goon;
	    }
	}
      
      
      printf("消息发送完成 at %lu\n", time(NULL));
      break;
    goon:
    }
  
}

int main(int argc, char agrv *[])
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
      printUsage();
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
