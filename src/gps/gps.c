/**
 * @file    gps.c
 * @brief   ATGM336H driver source file
 */
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "sys/log.h"
#include "src/gps/gps.h"
#include "src/drivers/uart.h"
#include "ext/mavlink/c_library_v2/common/mavlink.h"

static int uart_fd = 0;

static double gps_lat = 0.0;
static double gps_lon = 0.0;
static bool gpsValid = false;

static mavlink_message_t mav_msg;
static mavlink_status_t  mav_status;

/*	NHT - hs */
static double hs_rows[HS_GRID_ROWS][COORD_LIMITS] = {
    {10.747557000000000, 10.747717857142858}, 	/* row 0 */
    {10.747717857142858, 10.747878714285715}, 	/* row 1 */
    {10.747878714285715, 10.748039571428572}, 	/* row 2 */
    {10.748039571428572, 10.748200428571428}, 	/* row 3 */
    {10.748200428571428, 10.748361285714285}, 	/* row 4 */
    {10.748361285714285, 10.748522142857142}, 	/* row 5 */
    {10.748522142857142, 10.748674000000000} 	/* row 6 */
};
static double hs_columns[HS_GRID_COLUMNS][COORD_LIMITS] = {
	{106.71460300000000, 106.71477828571429},	/* column 0 */
	{106.71477828571429, 106.71495357142857},	/* column 1 */
	{106.71495357142857, 106.71512885714286},	/* column 2 */
	{106.71512885714286, 106.71530414285714},	/* column 3 */
	{106.71530414285714, 106.71547942857143},	/* column 4 */
	{106.71547942857143, 106.71565471428570},	/* column 5 */
	{106.71565471428570, 106.71583000000000}	/* column 6 */
};

/*	TDTU - us */
static double us_rows[US_GRID_ROWS][COORD_LIMITS] = {
    {10.730567000000000, 10.730719529411765}, 	/* row 0 */
    {10.730719529411765, 10.730872058823530}, 	/* row 1 */
    {10.730872058823530, 10.731024588235295}, 	/* row 2 */
    {10.731024588235295, 10.731177117647059}, 	/* row 3 */
    {10.731177117647059, 10.731329647058823}, 	/* row 4 */
    {10.731329647058823, 10.731482176470589}, 	/* row 5 */
    {10.731482176470589, 10.731634705882353}, 	/* row 6 */
    {10.731634705882353, 10.731787235294117}, 	/* row 7 */
    {10.731787235294117, 10.731939764705883}, 	/* row 8 */
    {10.731939764705883, 10.732092294117647}, 	/* row 9 */
    {10.732092294117647, 10.732244823529411}, 	/* row 10 */
    {10.732244823529411, 10.732397352941177}, 	/* row 11 */
    {10.732397352941177, 10.732549882352941}, 	/* row 12 */
    {10.732549882352941, 10.732702411764706}, 	/* row 13 */
    {10.732702411764706, 10.732854941176470}, 	/* row 14 */
    {10.732854941176470, 10.733007470588236}, 	/* row 15 */
    {10.733007470588236, 10.733160000000000} 	/* row 16 */
};
static double us_columns[US_GRID_COLUMNS][COORD_LIMITS] = {
	{106.69453400000000, 106.69475291666667},	/* column 0 */
	{106.69475291666667, 106.69497183333334},	/* column 1 */
	{106.69497183333334, 106.69519075000001},	/* column 2 */
	{106.69519075000001, 106.69540966666668},	/* column 3 */
	{106.69540966666668, 106.69562858333333},	/* column 4 */
	{106.69562858333333, 106.69584750000000},	/* column 5 */
	{106.69584750000000, 106.69606641666667},	/* column 6 */
	{106.69606641666667, 106.69628533333334},	/* column 7 */
	{106.69628533333334, 106.69650425000000}, 	/* column 8 */
	{106.69650425000000, 106.69672316666667},	/* column 9 */
	{106.69672316666667, 106.69694208333334},	/* column 10 */
	{106.69694208333334, 106.69716100000000},	/* column 11 */
	{106.69716100000000, 106.69737991666666},	/* column 12 */
	{106.69737991666666, 106.69759883333333},	/* column 13 */
	{106.69759883333333, 106.69781775000000}, 	/* column 14 */
	{106.69781775000000, 106.69803666666667},	/* column 15 */
	{106.69803666666667, 106.69825558333334},	/* column 16 */
	{106.69825558333334, 106.69847450000000},	/* column 17 */
	{106.69847450000000, 106.69869341666667},	/* column 18 */
	{106.69869341666667, 106.69891233333334}, 	/* column 19 */
	{106.69891233333334, 106.69913125000000},	/* column 20 */
	{106.69913125000000, 106.69935016666666},	/* column 21 */
	{106.69935016666666, 106.69956908333333},	/* column 22 */
	{106.69956908333333, 106.69978800000000}	/* column 23 */
};

void getGridPosition(char** key, int* row, int* col, double lat, double lon)
{
	if (lat >= hs_rows[FIRST_IDX][MIN_BOUND] && lat <= hs_rows[HS_LAST_ROW_IDX][MAX_BOUND] &&
	lon >= hs_columns[FIRST_IDX][MIN_BOUND] && lon <= hs_columns[HS_LAST_COL_IDX][MAX_BOUND]) {
		LOG_INF("Detected location: Nguyen Huu Tho Highschool");
		*key = LOCATION_NHT;

		for (int i = 0; i < HS_GRID_ROWS; i++) {
			if (lat >= hs_rows[i][MIN_BOUND] && lat < hs_rows[i][MAX_BOUND]) {
				*row = i;
				break;
			}
		}

		for (int i = 0; i < HS_GRID_COLUMNS; i++) {
			if (lon >= hs_columns[i][MIN_BOUND] && lon < hs_columns[i][MAX_BOUND]) {
				*col = i;
				break;
			}
		}
	}
	else if (lat >= us_rows[FIRST_IDX][MIN_BOUND] && lat <= us_rows[US_LAST_ROW_IDX][MAX_BOUND] &&
	lon >= us_columns[FIRST_IDX][MIN_BOUND] && lon <= us_columns[US_LAST_COL_IDX][MAX_BOUND]) {
		LOG_INF("Detected location: Ton Duc Thang University");
		*key = LOCATION_TDTU;
		
		for (int i = 0; i < US_GRID_ROWS; i++) {
			if (lat >= us_rows[i][MIN_BOUND] && lat < us_rows[i][MAX_BOUND]) {
				*row = i;
				break;
			}
		}

		for (int i = 0; i < US_GRID_COLUMNS; i++) {
			if (lon >= us_columns[i][MIN_BOUND] && lon < us_columns[i][MAX_BOUND]) {
				*col = i;
				break;
			}
		}
	}
	else {
		LOG_WRN("Coordinates out of range - retaining previous grid position");
	}
}

static void gpsHandleMavlinkMsg(mavlink_message_t *msg)
{
    switch (msg->msgid)
    {
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
            mavlink_global_position_int_t pos;
            mavlink_msg_global_position_int_decode(msg, &pos);

            gps_lat = pos.lat / 1e7;
            gps_lon = pos.lon / 1e7;
			gpsValid = true;
            break;
        }

        default:
            break;
    }
}

void gpsReadMavlink(void)
{
    uint8_t byte;
    int ret;

    while (1)
    {
        ret = read(uart_fd, &byte, 1);
        if (ret <= 0)
            break;

        if (mavlink_parse_char(MAVLINK_COMM_0, byte, &mav_msg, &mav_status))
        {
            gpsHandleMavlinkMsg(&mav_msg);
        }
    }
}

void getGpsCoordinates(double* lat, double* lon)
{
	if (lat == NULL || lon == NULL || !gpsValid) 
		return;

	*lat = gps_lat;
	*lon = gps_lon;
	gpsValid = false;
}

int GPS_uart_init(char* uart_file_path)
{
	uart_fd = uart_init(uart_file_path, B57600, false);
    if (uart_fd < 0) {
        return -1;
	}
    
	LOG_INF("GPS Initialization successful");
	return 0;
}
