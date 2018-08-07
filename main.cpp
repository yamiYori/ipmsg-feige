#include"chaos.h"
#include"definations.h"
#include"errno.h"
#include"fileManager.h"
#include"include/ipmsg.h"
#include"msgManager.h"
#include"packMaker.h"
#include"tcpReceiver.h"
#include"udpReceiver.h"
#include"usermanager.h"
#include<arpa/inet.h>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fcntl.h>
#include<net/if.h>
#include<netdb.h>
#include<netinet/in.h>
#include<pthread.h>
#include<pthread.h>
#include<pwd.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<utime.h>
#include<vector>

using std::vector;
static Chaos* chaos=0;
static UserManager* users=0;
static MsgManager* msgs=0;
static FileManager* filels=0;

int main(){
	chaos=Chaos::getInstance();
	if(unlikely(chaos==NULL)){
		printf("\e[31mFatel error:\e[0mChaos not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,chaos,errno);
		exit(1);
	}
	users=UserManager::getInstance();
	if(unlikely(users==NULL)){
		printf("\e[31mFatel error:\e[0mUserManager not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,users,errno);
		exit(1);
	}
	msgs=MsgManager::getInstance();
	if(unlikely(msgs==NULL)){
		printf("\e[31mFatel error:\e[0mMsgManager not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,msgs,errno);
	}
	filels=FileManager::getInstance();
	if(unlikely(filels==NULL)){
		printf("\e[31mFatel error:\e[0mFileManager not exist.\n");
		dpf("%s,%d,%p,%d\n",__FILE__,__LINE__,filels,errno);
	}

	pthread_t udpThd;
	pthread_t tcpThd;
	pthread_attr_t attr;
	int funcAns=pthread_attr_init(&attr);
	if(unlikely(funcAns!=0)){
		printf("\e[31mFatel error:\e[0mthread create error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,funcAns,errno);
		exit(1);
	}
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

	funcAns=pthread_create(&udpThd,&attr,thd_udpReceiver,NULL);
	if(unlikely(funcAns!=0)){
		printf("\e[31mFatel error:\e[0mthread create error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,funcAns,errno);
		exit(1);
	}
	funcAns=pthread_create(&tcpThd,&attr,thd_tcpReceiver,NULL);
	if(unlikely(funcAns!=0)){
		printf("\e[31mFatel error:\e[0mthread create error.\n");
		dpf("%s,%d,%d,%d\n",__FILE__,__LINE__,funcAns,errno);
		exit(1);
	}

	gethostname(users->myinfo.computername,100);
	struct passwd *pwd;
	pwd=getpwuid(getuid());
	strncpy(users->myinfo.sysusrname,pwd->pw_name,100);

	printf("waiting udp thread ready...\n");
	while(chaos->udpListen==false);
	printf("waiting tcp thread ready...\n");
	while(chaos->tcpListen==false);
	//broadcast...
	char buff[BUFFLEN];
	int opt=1;
	int pklen=mkHeartPack(buff,IPMSG_BR_ENTRY|IPMSG_BROADCASTOPT);
	int bcudpsock=chaos->udpsocket;

	struct sockaddr_in bcudpaddr;
	bzero(&bcudpaddr,sizeof(struct sockaddr_in));
	bcudpaddr.sin_family=AF_INET;
	bcudpaddr.sin_addr.s_addr=htonl(INADDR_BROADCAST);
	bcudpaddr.sin_port=htons(PORT);
	socklen_t bcudpaddr_len=sizeof(bcudpaddr);
	sprintf(buff+1,"1.feige.1");
	buff[10]=':';
	sendto(bcudpsock,buff+1,pklen-1,0,(struct sockaddr*)&bcudpaddr,bcudpaddr_len);
	sprintf(buff,"%s",vermagic);
	buff[10]=':';
	sendto(bcudpsock,buff,pklen,0,(struct sockaddr*)&bcudpaddr,bcudpaddr_len);

	if(1){		//console...
		char cmdbuf[BUFFLEN];
		char sendbuff[BUFFLEN];
		int sendlen=0;
		struct sockaddr_in c_addr;
		int sock=chaos->udpsocket;
		bzero(&c_addr,sizeof(c_addr));
		c_addr.sin_family=AF_INET;
		c_addr.sin_port=htons(PORT);
		socklen_t addr_len=sizeof(c_addr);

		while(chaos->programOK){
			msg_t tmpmsg;
			while(msgs->PopMsg(tmpmsg)==FUNCSUCCEED){
			char sztmpip[24];
			struct in_addr ip;
			ip.s_addr=tmpmsg.ip;
				strcpy(sztmpip,inet_ntoa(ip));
				printf("%s said:\n%s\n",sztmpip,tmpmsg.msg.c_str());
			}

			memset(cmdbuf,0,sizeof(cmdbuf));
			memset(sendbuff,0,sizeof(sendbuff));
			printf(">");
			fflush(stdin);
			fgets(cmdbuf,sizeof(cmdbuf),stdin);
			if(cmdbuf[0]==0)continue;
			if(cmdbuf[0]=='\n')continue;
			int curpos=0;
			char tmpbuf[BUFFLEN/16];
			sscanf(cmdbuf+curpos," %[^\n ]%n",tmpbuf,&curpos);

			if(strncmp(tmpbuf,"ls",3)				==0){
				list<user_t> tmplist;
				users->GetUserCopy(tmplist);
				for(auto person:tmplist){
					char tmpip[24];
					strcpy(tmpip,inet_ntoa(person.ip.sin_addr));
					printf("%-20s %-12s %-12s %-10s\n",tmpip,person.sysusrname,
								person.computername,person.usrname);
				}
			}else if(strncmp(tmpbuf,"la",3)			==0){
				char ipbuf[20];
				sscanf(cmdbuf+curpos," %[0-9.]",ipbuf);
				in_addr_t tarip=inet_addr(ipbuf);
				if(tarip==-1){
					printf("Invaild ip address.\n");
					dpf("%d,%s,%d\n",__LINE__,__FILE__,errno);
				}else{
					user_t tar;
					int findans=users->Finduser(tar,tarip);
					if(findans==FUNCSUCCEED){
						printf("%-20s:%s\n","IP  Address",ipbuf);
						printf("%-20s:%s\n","System  Username",tar.sysusrname);
						printf("%-20s:%s\n","Computer Username",tar.computername);
						printf("%-20s:%s\n","Username",tar.usrname);
						printf("%-20s:%s\n","Groupname",tar.grpname);
						printf("%-20s:%s\n","Mac Address",tar.macaddr);
						printf("%-20s:%s\n","Telephone Number",tar.phone);
						printf("%-20s:%s\n","E-mail Address",tar.mail);
						printf("%-20s:%s\n","Share Printer Name",tar.printer);
						printf("%-20s:%s\n","Icon Number",tar.icon);
						char statusbuf[16]={0};
						switch(tar.icon[0]){
						case '0':
							sprintf(statusbuf,"Online");
							break;
						case '1':
							sprintf(statusbuf,"Busy");
							break;
						case '2':
							sprintf(statusbuf,"Away from keyboard");
							break;
						}
						printf("%-20s:%s\n","User Status",statusbuf);
						printf("%-20s:%s\n","Sign",tar.sign);
					}else{
						printf("User %s not found.\n",ipbuf);
					}
					printf("\n");
				}

			}else if(strncmp(tmpbuf,"setmyinfo",10)	==0){
				char infobuf[1000];
				fflush(stdin);
				infobuf[0]=0;
				printf("Please input your username.Input nothing to keep current value.\n(current is \e[32m%2s\e[0m)\e[47;32m\n",users->myinfo.usrname);
				scanf(" %[^\n]",infobuf);
				if(infobuf[0]!='\n'){
					strncpy(users->myinfo.usrname,infobuf,100);
				}
				printf("\e[0m");

				fflush(stdin);
				infobuf[0]=0;
				printf("Please input your groupname.Input nothing to keep current value.\n(current is \e[32m%2s\e[0m)\e[47;32m\n",users->myinfo.grpname);
				scanf(" %[^\n]",infobuf);
				if(infobuf[0]!='\n'){
					strncpy(users->myinfo.grpname,infobuf,100);
				}
				printf("\e[0m");

				fflush(stdin);
				infobuf[0]=0;
				printf("Please input your phone number.Input nothing to keep current value.\n(current is \e[32m%2s\e[0m)\e[47;32m\n",users->myinfo.phone);
				scanf(" %[^\n]",infobuf);
				if(infobuf[0]!='\n'){
					strncpy(users->myinfo.phone,infobuf,20);
				}
				printf("\e[0m");

				fflush(stdin);
				infobuf[0]=0;
				printf("Please input your e-mail.Input nothing to keep current value.\n(current is \e[32m%2s\e[0m)\e[47;32m\n",users->myinfo.mail);
				scanf(" %[^\n]",infobuf);
				if(infobuf[0]!='\n'){
					strncpy(users->myinfo.mail,infobuf,100);
				}
				printf("\e[0m");

				fflush(stdin);
				infobuf[0]=0;
				printf("Please select your icon(0-31).Input nothing to keep current value.\n(current is \e[32m%2s\e[0m)\e[47;32m\n",users->myinfo.icon);
				scanf(" %[^\n]",infobuf);
				if(infobuf[0]!='\n'){
					strncpy(users->myinfo.icon,infobuf,10);
				}
				printf("\e[0m");

				fflush(stdin);
				infobuf[0]=0;
				printf("Please input your sign.Input nothing to keep current value.\n(current is \e[32m%2s\e[0m)\e[47;32m\n",users->myinfo.sign);
				scanf(" %[^\n]",infobuf);
				if(infobuf[0]!='\n'){
					strncpy(users->myinfo.sign,infobuf,1000);
				}
				printf("\e[0m");
				strncpy(users->myinfo.netstatus,"10000001",10);
				strncpy(users->myinfo.space02,"5",10);
				pklen=mkHeartPack(buff,IPMSG_BR_ENTRY);
				sendto(bcudpsock,buff,pklen,0,(struct sockaddr*)&bcudpaddr,bcudpaddr_len);
				printf("Set personal info complete!\n");
			}else if(strncmp(tmpbuf,"send",5)		==0){
				char ipbuf[20];
				int optstart;
				sscanf(cmdbuf+curpos," %[^\n ]%n",ipbuf,&optstart);
				in_addr_t tarip=inet_addr(ipbuf);
				if(tarip==-1){
					printf("Invaild ip address.\n");
					dpf("%d,%s,%d\n",__LINE__,__FILE__,errno);
				}else{
					char msgbuf[BUFFLEN];
					sscanf(cmdbuf+curpos+optstart," %[^\n]%n",msgbuf,&curpos);
					c_addr.sin_addr.s_addr=tarip;
					sendlen=mkHeader(sendbuff,IPMSG_SENDMSG);
					sendlen+=sprintf(sendbuff+sendlen,msgbuf);
					sendlen+=sprintf(sendbuff+sendlen,"%s",fontmagic);
					sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
					printf("\n");
				}
			}else if(strncmp(tmpbuf,"refresh",8)	==0){
				users->UserIllness();
				pklen=mkHeartPack(buff,IPMSG_BR_ENTRY|IPMSG_BROADCASTOPT);
				sprintf(buff+1,"1.feige.1");
				buff[10]=':';
				sendto(bcudpsock,buff+1,pklen-1,0,(struct sockaddr*)&bcudpaddr,bcudpaddr_len);
				sprintf(buff,"%s",vermagic);
				buff[10]=':';
				sendto(bcudpsock,buff,pklen,0,(struct sockaddr*)&bcudpaddr,bcudpaddr_len);

				printf("BroadCast to %s\n",inet_ntoa(bcudpaddr.sin_addr));
				printf("Refreshing");
				fflush(stdout);
				sleep(1);
				printf(".");
				fflush(stdout);
				sleep(1);
				printf(".");
				fflush(stdout);
				sleep(1);
				printf(".");
				fflush(stdout);
				sleep(1);
				list<user_t> tmpusr;
				users->GetUserCopy(tmpusr);
				auto i=tmpusr.begin();
				auto z=tmpusr.end();
				for(;i!=z;i++){
					if(i->healthy==false){
						users->Deluser(i->ip);
					}
				}
				printf("\n");
			}else if(strncmp(tmpbuf,"lsfile",7)		==0){
				list<recvfile_t> tmprecvls;
				filels->GetrecvCopy(tmprecvls);
				for(auto i:tmprecvls){
					struct sockaddr_in tmpaddr;
					tmpaddr.sin_addr.s_addr=i.tarIp;
					char tmpip[24];
					strcpy(tmpip,inet_ntoa(tmpaddr.sin_addr));
					int typeindex=((i.filetype>0)&&(i.filetype<11))?i.filetype:0;
					printf("%20s-%-12u %15s %s\n",tmpip,i.fileno,filetype_str[i.filetype],i.filename.c_str());
				}
				printf("Press any key to continue...");
				getchar();

				list<sendfile_t> tmpsendls;
				filels->GetsendCopy(tmpsendls);
				for(auto i:tmpsendls){
					struct sockaddr_in tmpaddr;
					tmpaddr.sin_addr.s_addr=i.tarIp;
					char tmpip[24];
					strcpy(tmpip,inet_ntoa(tmpaddr.sin_addr));
					int typeindex=((i.filetype>0)&&(i.filetype<11))?i.filetype:0;
					printf("%20s-%-12u %15s %s\n",tmpip,i.fileno,filetype_str[i.filetype],i.filename.c_str());
				}
				printf("Press any key to continue...");
				getchar();
			}else if(strncmp(tmpbuf,"sendfile",9)	==0){
				char raw_ip[20];
				char fnamebuf[BUFFLEN];
				char extra=0;
				struct in_addr tmpip;
				int optindex=0;
				sscanf(cmdbuf+curpos," %[^\n ]%n",raw_ip,&optindex);
				if(inet_aton(raw_ip,&tmpip)!=-1){
					sscanf(cmdbuf+curpos+optindex," %[^\n ]%n",fnamebuf,&optindex);
					if(fnamebuf[0]=='-'){
						extra=fnamebuf[1];
						sscanf(cmdbuf+curpos," %[^\n ]%n",fnamebuf,&curpos);
					}
					struct stat statbuf;
					int statans=stat(fnamebuf,&statbuf);
					puts(fnamebuf);
					if(statans==0){
						char nakedfnamebuf[BUFFLEN];
						char* tmp=strrchr(fnamebuf,'/');
						if(tmp!=NULL){
							strcpy(nakedfnamebuf,strrchr(fnamebuf,'/')+1);
						}else{
							strcpy(nakedfnamebuf,fnamebuf);
						}
						int filetp=0;
						if(S_ISREG(statbuf.st_mode)){
							filetp=IPMSG_FILE_REGULAR;
						}else if(S_ISDIR(statbuf.st_mode)){
							filetp=IPMSG_FILE_DIR;
						}
						if(filetp){
							unsigned packno;
							sendlen=mkHeader(sendbuff,IPMSG_SENDMSG|IPMSG_FILEATTACHOPT|IPMSG_SENDCHECKOPT,&packno);
							sendlen+=sprintf(sendbuff+sendlen,"")+1;
							sendlen+=sprintf(sendbuff+sendlen,"%u:%s:%x:0:%u:%u:%u:%u:",
								packno-1,nakedfnamebuf,statbuf.st_size,filetp,
								statbuf.st_atime,statbuf.st_mtime,statbuf.st_ctime)+1;
							sendfile_t sendnode;
							sendnode.tarIp=tmpip.s_addr;
							sendnode.fileno=packno-1;
							sendnode.filename=nakedfnamebuf;
							sendnode.filesize=statbuf.st_size;
							sendnode.filelocate=fnamebuf;
							sendnode.filetype=filetp;

							if(filels->Addsend(sendnode)==FUNCSUCCEED){
								c_addr.sin_addr.s_addr=tmpip.s_addr;
								sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
							}else{
								printf("Unknown error,please retry.\n");
							}
						}
					}else{
						printf("Read file information error.\n");
						dpf("%d,%s,%d\n",__LINE__,__FILE__,errno);
					}
				}else{
					printf("Invalid ip address!\n");
				}

			}else if(strncmp(tmpbuf,"recvfile",9)	==0){		//From this function, A rebuild occured, Code style changed
				char raw_ip[20];
				unsigned fileno;
				struct in_addr tmpip;
				char other[BUFFLEN]={0};
				char optlocate[BUFFLEN]={0};
				sscanf(cmdbuf+curpos," %[^\n-]-%u %[%^\n]",raw_ip,&fileno,other);

				if(inet_aton(raw_ip,&tmpip)==-1){
					printf("Invalid ip address!\n");
					continue;
				}
				if((other[0]=='-')&&((other[1]=='f')||(other[1]=='F'))){
					sscanf(other+2," %[^\n]",optlocate);
				}
				recvfile_t recvnode;
				if(filels->Findrecv(fileno,tmpip.s_addr,recvnode)==FUNCFAILED){
					printf("Pending file not found!\n");
					continue;
				}
				if(optlocate[0]==0){
					sprintf(optlocate,"./%s",recvnode.filename.c_str());
				}

				int sockfd=socket(AF_INET,SOCK_STREAM,0);					//NC
				if(sockfd==-1){
					printf("Connect failed!\n");
					dpf("%d,%s,%d\n",__LINE__,__FILE__,errno);
					continue;
				}
				struct sockaddr_in addr;
				addr.sin_family=AF_INET;
				addr.sin_addr.s_addr=tmpip.s_addr;
				addr.sin_port=htons(PORT);
				int len=sizeof(addr);

				int newsockfd=connect(sockfd,(struct sockaddr *)&addr,len);	//NC

				if(newsockfd==-1){
					printf("Connect failed!\n");
					dpf("%d,%s,%d\n",__LINE__,__FILE__,errno);
					close(sockfd);
					continue;
				}
				char sendbuff[BUFFLEN];
				int sendlen;
				char recvbuff[BUFFLEN];
				int recvlen;
				int filetp;
				if(recvnode.filetype==IPMSG_FILE_REGULAR){
					filetp=IPMSG_GETFILEDATA;
					FILE * fp;
					if((fp=fopen(optlocate,"wb"))==NULL){				//NC
						printf("Open %s failed:%d.\n",optlocate,errno);
						close(sockfd);
						continue;
					}

					sendlen=mkHeader(sendbuff,filetp);
					sendlen+=sprintf(sendbuff+sendlen,"%lx:%lx:0:",recvnode.fileno+1,recvnode.fileno);
					send(sockfd,sendbuff,sendlen,0);
					char *filebuff=(char *)malloc(recvnode.filesize);				//NC
					if(filebuff==NULL){
						printf("Recving failed.Not enough memory.\n");
						close(sockfd);
						fclose(fp);
					}

					bool transok=false;
					while(transok==false){
						recvlen=recv(sockfd,filebuff,sizeof(filebuff),0);
						if(recvlen<=0){
							transok=true;
						}else{
							fwrite(filebuff,recvlen,1,fp);
						}
					}
					fclose(fp);
					free(filebuff);
					filels->Delrecv(fileno,tmpip.s_addr);

				}else if(recvnode.filetype==IPMSG_FILE_DIR){
					filetp=IPMSG_GETDIRFILES;
					int mkans=mkdir(optlocate,0755);
					if(mkans==-1){
						printf("Create directory failed:%d.",errno);
						dpf("%d,%s,%d\n",__LINE__,__FILE__,errno);
						continue;
					}
					sendlen=mkHeader(sendbuff,filetp);
					sendlen+=sprintf(sendbuff+sendlen,"%lx:%lx:0:",recvnode.fileno+1,recvnode.fileno);
					send(sockfd,sendbuff,sendlen,0);

					bool transok=false;
					while(transok==false){
						recvlen=recv(sockfd,recvbuff,BUFFLEN,0);
						if(recvlen<=0){
							transok=true;
						}else{
							utime(optlocate,NULL);
							transok=true;//FIXME: I ignored all file under a directory.Someday this sentence need to be removed.
						}
					}
					filels->Delrecv(fileno,tmpip.s_addr);
				}else{
					printf("Unsupported file type.\n");
				}
				close(sockfd);
			}else if(strncmp(tmpbuf,"refuse",7)		==0){
				char raw_ip[20];
				unsigned fileno;
				struct in_addr tmpip;
				char other[BUFFLEN]={0};
				char optlocate[BUFFLEN]={0};
				sscanf(cmdbuf+curpos,"  %[^\n-]-%u",raw_ip,&fileno);
				if(inet_aton(raw_ip,&tmpip)==-1){
					printf("Invalid ip address!\n");
					continue;
				}
				recvfile_t recvnode;
				if(filels->Findrecv(fileno,tmpip.s_addr,recvnode)==FUNCFAILED){
					printf("Pending file not found!\n");
					continue;
				}
				filels->Delrecv(fileno,tmpip.s_addr);
				sendlen=mkHeader(sendbuff,IPMSG_EX_REFUSEFILE);
				sendlen+=sprintf(sendbuff+sendlen,"%lx",recvnode.fileno+1)+1;
				sendlen+=sprintf(sendbuff+sendlen,"");
				sendlen+=sprintf(sendbuff+sendlen,"%lx",recvnode.fileno);
				c_addr.sin_addr.s_addr=tmpip.s_addr;
				sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
				printf("\n");

			}else if(strncmp(tmpbuf,"cancel",7)		==0){
				char raw_ip[20];
				unsigned fileno;
				struct in_addr tmpip;
				char other[BUFFLEN]={0};
				char optlocate[BUFFLEN]={0};
				sscanf(cmdbuf+curpos," %[^\n-]-%u",raw_ip,&fileno);
				if(inet_aton(raw_ip,&tmpip)==-1){
					printf("Invalid ip address!\n");
					continue;
				}
				sendfile_t sendnode;
				if(filels->Findsend(fileno,tmpip.s_addr,sendnode)==FUNCFAILED){
					printf("Pending file not found!\n");
					continue;
				}
				filels->Delsend(fileno,tmpip.s_addr);
				sendlen=mkHeader(sendbuff,IPMSG_EX_CANCELFILE);
				sendlen+=sprintf(sendbuff+sendlen,"%lx",sendnode.fileno+1)+1;
				sendlen+=sprintf(sendbuff+sendlen,"");
				sendlen+=sprintf(sendbuff+sendlen,"%lx",sendnode.fileno);
				c_addr.sin_addr.s_addr=tmpip.s_addr;
				sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
				printf("\n");
			}else if(strncmp(tmpbuf,"listen",7)		==0){
				printf("Unimplemented feature.\n");
				continue;
			}else if(strncmp(tmpbuf,"system",7)		==0){
				char other[BUFFLEN]={0};
				sscanf(cmdbuf+curpos," %[^\n]",other);
				system(other);
				printf("\n");
			}else if(strncmp(tmpbuf,"setnetwork",11)==0){
				struct ifconf ifconf;
				struct ifreq *ifreq;
				vector<unsigned> arrayip;
				char buf[512];
				ifconf.ifc_len=512;
				ifconf.ifc_buf=buf;
				ioctl(sock,SIOCGIFCONF,&ifconf);
				ifreq=(struct ifreq*)ifconf.ifc_buf;
				struct ifreq*ifreqbackup=ifreq;
				printf("Founded network device number:%d\n",(ifconf.ifc_len/sizeof (struct ifreq)));
				for (int i=(ifconf.ifc_len/sizeof (struct ifreq)); i>0; i--){
					if(ifreq->ifr_flags == AF_INET){ //for ipv4
						printf("name =[%s]\n" , ifreq->ifr_name);
						printf("local addr = [%s]\n" ,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
						arrayip.push_back(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr.s_addr);
						ifreq++;
					}
				}
				printf("Select your device[1-%d]:",arrayip.size());
				int select=0;
				scanf(" %d",&select);
				fflush(stdin);
				if((select>=1)&&(select<=arrayip.size())){
					printf("Reset network device success!\n");
					bcudpaddr.sin_addr.s_addr=arrayip[select-1]|0xFF000000UL;
					struct in_addr tmpip;
					tmpip.s_addr=arrayip[select-1];
					strncpy(users->myinfo.ipaddr,inet_ntoa(tmpip),20);
					int macsel=select;
					while(macsel>1){
						macsel--;
						ifreqbackup++;
					}
					sprintf(users->myinfo.macaddr,"%02X-%02X-%02X-%02X-%02X-%02X",
							(unsigned char)ifreqbackup->ifr_hwaddr.sa_data[0],
							(unsigned char)ifreqbackup->ifr_hwaddr.sa_data[1],
							(unsigned char)ifreqbackup->ifr_hwaddr.sa_data[2],
							(unsigned char)ifreqbackup->ifr_hwaddr.sa_data[3],
							(unsigned char)ifreqbackup->ifr_hwaddr.sa_data[4],
							(unsigned char)ifreqbackup->ifr_hwaddr.sa_data[5]);
				}else{
					printf("Invalid device number!\n");
				}
			}else if(strncmp(tmpbuf,"exit",5)		==0){
				chaos->programOK=false;
			}else if(strncmp(tmpbuf,"help",5)		==0){
				printf("*****************************Help Menu****************************\n");
				printf(" ls-------------------------------------------list all online usr \n");
				printf(" la [ip]-----------------------------------list detail infomation \n");
				printf(" refresh---------------------------------------refresh online usr \n");
				printf(" **************************************************************** \n");
				printf(" send [ip] [message]--------------------------------begin to chat \n");
				printf(" sendfile [ip] [filename] -----------------send file or directory \n");
				printf(" recvfile [ip]-[fileno] {-fF [newname]}----recv file or directory \n");
				printf(" lsfile-------------------------------------list all pending file \n");
				printf(" refuse [ip]-[fileno]-----------------------refuse to accept file \n");
				printf(" cancel [ip]-[fileno]-------------------------cancel to send file \n");
				printf(" **************************************************************** \n");
				printf(" setmyinfo---------------------------------------------set myinfo \n");
				printf(" setnetwork---------------------------------select network device \n");
				printf(" help---------------------------------------------------help info \n");
				printf(" exit-------------------------------------------------quit system \n");
				printf(" **************************************************************** \n");
				printf(" system [command]------------------------------call shell command \n");
				printf("*****************************Help Menu*************************** \n");

			}else{
				printf("Unknown command!\nUse \"help\" to get more info.\n");
			}

		}
		sendlen=mkHeader(sendbuff,IPMSG_BR_EXIT);
		sendlen+=sprintf(sendbuff+sendlen,"%s",users->myinfo.usrname);
		sendto(bcudpsock,sendbuff,sendlen,0,(struct sockaddr*)&bcudpaddr,bcudpaddr_len);
		list<user_t> tmplist;
		users->GetUserCopy(tmplist);
		for(auto person:tmplist){
			c_addr.sin_addr.s_addr=person.ip.sin_addr.s_addr;
			sendto(sock,sendbuff,sendlen,0,(struct sockaddr*)&c_addr,addr_len);
		}
		printf("Exiting...\n");
		pthread_join(udpThd,NULL);
		pthread_join(tcpThd,NULL);

	}
	if(0){		//TODO:XCUI...
	}
	return 0;
}

