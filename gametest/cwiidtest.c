#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))

cwiid_err_t err;
void err(cwiid_wiimote_t *wiimote, const char *s, va_list ap)
{
	if (wiimote) printf("%d:", cwiid_get_id(wiimote));
	else printf("-1:");
	vprintf(s, ap);
	printf("\n");
}

void set_rpt_mode(cwiid_wiimote_t *wiimote, unsigned char rpt_mode)
{
	if (cwiid_set_rpt_mode(wiimote, rpt_mode)) {
		fprintf(stderr, "Error setting report mode\n");
	}
}

time_t elapsedTime(struct timeval startTime, struct timeval endTime) {
    time_t diffsec = difftime(endTime.tv_sec, startTime.tv_sec);
    suseconds_t diffsub = endTime.tv_usec - startTime.tv_usec;
    if(diffsub < 0) {
        diffsec--;
        diffsub = (endTime.tv_usec+1000000) - startTime.tv_usec;
    }
    uint16_t diffmsec = diffsub / 1000;
    time_t diff = (diffsec * 1000) + diffmsec;
    return diff;
}

int main(void){
	cwiid_wiimote_t *wiimote;	/* wiimote handle */
	struct cwiid_state state;	/* wiimote state */
	bdaddr_t bdaddr;	/* bluetooth device address */

    unsigned char led_state = 0;
    unsigned char rpt_mode = 0;

    cwiid_set_err(err);

    bdaddr = *BDADDR_ANY;

	printf("Put Wiimote in discoverable mode now (press 1+2)...\n");
	if (!(wiimote = cwiid_open(&bdaddr, 0))) {
		fprintf(stderr, "Unable to connect to wiimote\n");
		return -1;
	}

    sleep(1);

    printf("get button\n");
	toggle_bit(rpt_mode, CWIID_RPT_BTN);
	set_rpt_mode(wiimote, rpt_mode);
    //sleep(3);
    struct timeval startTime, endTime;

    while(1){
        gettimeofday(&startTime, NULL);
        if (cwiid_get_state(wiimote, &state)) {
            fprintf(stderr, "Error getting state\n");
        }
        printf("Buttons: ");
        if (state.buttons & CWIID_BTN_LEFT){
            printf("←");
        }
        if (state.buttons & CWIID_BTN_RIGHT){
            printf("→");
        }
        if (state.buttons & CWIID_BTN_UP){
            printf("↑");
        }
        if (state.buttons & CWIID_BTN_DOWN){
            printf("↓");
        }
        printf("\n");

        if(state.buttons==4){
            break;
        }
        gettimeofday(&endTime, NULL);
        time_t diff = elapsedTime(startTime, endTime);
        printf("%s elapsed time[ms]=%ld\n",__func__, diff);
        usleep(50000);
    }

    sleep(1);
    printf("disconnect\n");

	if (cwiid_close(wiimote)) {
		fprintf(stderr, "Error on wiimote disconnect\n");
		return -1;
	}
    return 0;
}