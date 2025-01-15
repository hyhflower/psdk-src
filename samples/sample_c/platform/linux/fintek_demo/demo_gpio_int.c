#include "fintek_api.h"
#include <pthread.h>

#define FINTEK_GPIO_INT_TRIGGER_PATH \
	"/sys/bus/platform/devices/fintek-gpio-int/irq-%d"
#define FORCE_GROUP_MODE 0
#define MAX_SET 4
#define MAX_GPIO_COUNT (1 + 8) //irq + gpio1 + gpio2 ...
#define INIT_VALUE 0xFF
#define NEED_LOCK 1

static pthread_attr_t pthread_attr;
static int m_fd[MAX_SET];
static pthread_t m_tid[MAX_SET];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int m_idx[MAX_SET][MAX_GPIO_COUNT];
unsigned int m_irq[MAX_SET];
sFintek_sio_data sio_data;

#define SECOND (1000000LL)
#define MSECOND (1000LL)

static void time_start(struct timeval *tv)
{
	gettimeofday(tv, NULL);
}

static time_t time_delta(struct timeval *start, struct timeval *stop)
{
	time_t secs, usecs;

	secs = stop->tv_sec - start->tv_sec;
	usecs = stop->tv_usec - start->tv_usec;
	if (usecs < 0) {
		secs--;
		usecs += SECOND;
	}

	return secs * SECOND + usecs;
}

static time_t time_stop(struct timeval *begin)
{
	struct timeval end;
	gettimeofday(&end, NULL);
	return time_delta(begin, &end);
}

void usage(int argc, char *argv[])
{
	fprintf(stderr, "\n\n\n\nFintek GPIO interrupt Demo:\n");
	fprintf(stderr, "%s <pin,irq> ...\n\n", argv[0]);
	fprintf(stderr, "Param List:\n");
	fprintf(stderr, "\tpin: number of GPIO idx, like GPIO12, idx=0x12\n");
	fprintf(stderr, "\tirq: irq channel\n\n");
	fprintf(stderr, "\texample: %s 0x80,5\n\n", argv[0]);
	fprintf(stderr, "Up to eight sets of GPIO.\n");
}

void lock()
{
#if NEED_LOCK
	pthread_mutex_lock(&mutex);
#endif
}

void unlock()
{
#if NEED_LOCK
	pthread_mutex_unlock(&mutex);
#endif
}

void *do_waiting_int(void *arg_irq)
{
	int irq = *((int *)arg_irq);
	int fd;
	int cnt = 0;
	int size;
	unsigned int data, status;
	char ch;
	int i, j;
	int idx[MAX_GPIO_COUNT - 1];
	unsigned char buf[512];
	struct timeval start, end;
	int flag = 0;

	for (i = 0; i < MAX_GPIO_COUNT - 1; i++)
		idx[i] = INIT_VALUE;

	for (i = 0; i < MAX_SET; ++i) {
		for (j = 1; j < MAX_GPIO_COUNT; ++j) {
			if ((m_idx[i][j] != INIT_VALUE) &&
			    (m_idx[i][0] == irq)) {
				idx[j - 1] = m_idx[i][j];
				status |= BIT(m_idx[i][j] & 0x0f);
			}
		}
	}

	while (1) {
		sprintf(buf, FINTEK_GPIO_INT_TRIGGER_PATH, irq);
		fd = open(buf, O_RDONLY);
		if (fd < 0) {
			printf("open failed\n");
			return 0;
		}

		while (1) {
			flag = 0;
			size = read(fd, &ch, 1);
			if (size < 0) {
				perror("read");
				break;
			}

			size = lseek(fd, SEEK_SET, 0);
			if (size < 0) {
				perror("lseek");
				break;
			}

			lock();
			printf("in\n");

			while (!flag) {
				flag = 1;
				CHECK_RET_TYPE(_GetGpioIntStatus(idx[0] >> 4,
								 &data),
					       void *);
				printf("Int Status data: 0x%x\n",data);
				CHECK_RET_TYPE(_ClearGpioIntStatus(idx[0] >> 4,//BIT(idx[0] &0x0f)
								   data),
					       void *);

				for (i = 0; i < MAX_GPIO_COUNT - 1; i++) {
					if (idx[i] != INIT_VALUE) {
						if ((data & BIT(idx[i] &
								0x0f)) == 0) {
							printf("out\n");
							flag &= 1;
							continue;
						} else {
							flag &= 0;
						}
						CHECK_RET_TYPE(
							_GetGpioInputDataIdx(
								idx[i], &data),
							void *);
						printf("GPIO%02x: cnt: %d, data: %d\n",
						       idx[i], cnt++, data);
					}
				}
			}
			unlock();
		}
		close(fd);
	}
	return NULL;
}

int gpio_int_demo(int argc, char *argv[])
{
	unsigned int data = 0;
	int status;
	int i, j;
	unsigned int idx, irq, idx_num;
	unsigned int pull_mode;

	if (argc < 2 || argc > MAX_GPIO_COUNT) {
		usage(argc, argv);
		exit(3);
	}

	for (i = 0; i < MAX_SET; i++)
		for (j = 0; j < MAX_GPIO_COUNT; j++)
			m_idx[i][j] = INIT_VALUE;

	for (i = 0; i < MAX_GPIO_COUNT && i < argc - 1; ++i) {
		sscanf(argv[i + 1], "%x,%d", &idx, &irq);
		if (i == 0) {
			m_idx[i][0] = irq;
			m_idx[i][1] = idx;
			m_irq[i] = irq;
		} else {
			for (j = 0; j < MAX_SET; j++) {
				if (irq == m_idx[j][0]) {
					idx_num = 0;
					while (idx_num < MAX_GPIO_COUNT &&
					       m_idx[j][idx_num] !=
						       INIT_VALUE) {
						idx_num++;
					}
					m_idx[j][idx_num] = idx;
					break;
				} else if ((irq != m_idx[j][0]) &&
					   (m_idx[j][0] == INIT_VALUE)) {
					m_idx[j][0] = irq;
					m_irq[i] = irq;
					m_idx[j][1] = idx;
					break;
				}
			}
		}

		if (!irq) {
			printf("irq can't be 0\n");
			usage(argc, argv);
			exit(3);
		}
	}

	for (i = 0; i < MAX_SET && i < argc - 1; ++i) {
		for (j = 1; j < MAX_GPIO_COUNT; ++j) {
			if (m_idx[i][j] != INIT_VALUE) {
				CHECK_RET(_EnableGPIO(m_idx[i][j],
						      eGPIO_Mode_Enable));
				printf("idx: 0x%x => set:%x, count:%x, irq: %x\n",
				       m_idx[i][j], (m_idx[i][j] >> 4) & 0xf,
				       (m_idx[i][j] >> 0) & 0xf, m_irq[i]);

				CHECK_RET(_SetGpioOutputEnableIdx(
					m_idx[i][j], eGPIO_Direction_In));
				printf("Dir: 0x%x => %s\n", data, "IN");

				// pull high if available (not error report)
				_SetGpioPullMode(m_idx[i][j], eGPIO_Pull_High);

				status = _GetGpioPullMode(m_idx[i][j], &pull_mode);
				if (!status) {
					switch (pull_mode) {
					case eGPIO_Pull_Low:
						printf("Pull mode: pull low\n");
						break;
					case eGPIO_Pull_High:
						printf("Pull mode: pull high\n");
						break;
					case eGPIO_Pull_Disable:
						printf("Pull mode: pull disable\n");
						break;
					default:
					case eGPIO_Pull_Invalid:
						printf("Pull mode: invalid\n");
						break;
					}

				} else {
					printf("Not support pull mode setting\n");
				}

				CHECK_RET(_DisableGpioInt(m_idx[i][j] >> 4,
							  BIT(m_idx[i][j] &
							      0x0f)));
				CHECK_RET(_ClearGpioIntStatus(m_idx[i][j] >> 4,
							      BIT(m_idx[i][j] &
								  0x0f)));
				CHECK_RET(_EnableGpioInt(
					m_idx[i][j] >> 4,
					BIT(m_idx[i][j] & 0x0f), m_irq[i]));
			}
		}
	}

	// do start pthreads
	pthread_attr_init(&pthread_attr);

	for (i = 0; i < MAX_SET && i < argc - 1; ++i) {
		if (m_idx[i][0] != INIT_VALUE)
			pthread_create(&m_tid[i], &pthread_attr,
				       do_waiting_int, &m_irq[i]);
	}

	return 0;
}

int gpio_DeInit()
{
	int i;
	int argc=2;
	// waiting end
	for (i = 0; i < MAX_SET && i < argc - 1; ++i) {
		if (m_idx[i][0] != INIT_VALUE)
			pthread_join(m_tid[i], NULL);
	}

	fflush(stdout);

	DeactiveSIO(sio_data.ic_port);

	return 0;
}

int main(int argc,char *argv[])
{
	int nRet = 0;
	char chppBuf[2][32]={0};
	char *ppBuf[2];

	/* Please set this to 1 enable more debug message */
	set_debug(1);

	fprintf(stderr, "argc=%d,FINTEK Lib Version: %s\n",argc, getFintekLibVersion());

	if (nRet = init_fintek_sio(eSIO_TYPE_F81966, 0, &sio_data)) {
		fprintf(stderr, "init_fintek_sio error\n");
		//exit(3);
		return 0;
	}
	printf("sio data port:%d,key:%d\n",sio_data.ic_port, sio_data.key);

	ActiveSIO(sio_data.ic_port, sio_data.key);

	ppBuf[0]=chppBuf[0];
	ppBuf[1]=chppBuf[1];
	snprintf(chppBuf[0], sizeof(chppBuf[0]), "name");
	snprintf(chppBuf[1], sizeof(chppBuf[1]), "0x85,5");
	/* GPIO interrupt test, need driver */
	gpio_int_demo(argc,ppBuf);

	gpio_DeInit();

	return 0;
}
