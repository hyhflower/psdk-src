#include "fintek_api.h"

int main(int argc, char *argv[])
{
	int nRet, i = 0;
	psFintek_sio_data sio_data, next;

	/* Please set this to 1 enable more debug message */
	set_debug(0);

	fprintf(stderr, "FINTEK Lib Version: %s\n", getFintekLibVersion());

	nRet = get_device_list(eSIO_TYPE_F81968, &sio_data);
	//nRet = get_device_list(eSIO_TYPE_F75114, &sio_data);

	printf("nRet: %d\n", nRet);

	while (sio_data) {
		printf("  idx: %d, type: %d\n", i++, sio_data->ic_type);

		if (sio_data->ic_type >= eSIO_TYPE_HID) {
			printf("    path_name: %s\n", sio_data->path_name);
			printf("    device_arch: %s\n", sio_data->device_arch);
		} else {
			printf("    ic_port: %x\n", sio_data->ic_port);
			printf("    key: %x\n", sio_data->key);
		}
		next = sio_data->next;
		free(sio_data);
		sio_data = next;
	}

	return 0;
}
