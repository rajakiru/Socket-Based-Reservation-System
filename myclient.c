#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFERSIZE 512
#define MAX_YR 9999
#define MIN_YR 2000

typedef struct
{
    int yyyy;
    int mm;
    int dd;
} Date;

typedef struct
{
  char *flt_line;
} flight_lines;

// structure to save the flight details
typedef struct FL_details
{
    char *FLight_id;
    char *Timing;
    char *Timing2;
    int seats_available_e;
    int total_seats_e;
    float fare_economy;
    int seats_available_b;
    int total_seats_b;
    float fare_business;
} flight_details;

// structure to save the traveller details.
typedef struct TRaveller
{
    char *name;
    int age;
    char *fl_id;
    char *class;
    float conv_seat_price;
    float seat_price;
    char *source;
    char *destination;
    char *departure;
    char *arrival;
    Date getDate;
    long int mobile_no;
} traveller_details;

char* receiveMsgFromServer(int sock_fd) {
    int rbuf = 0;
    int n = read(sock_fd, &rbuf, sizeof(int));
    if(n <= 0) {
        shutdown(sock_fd, SHUT_WR);
        return NULL;
    }
    char *str = (char*)malloc(rbuf*BUFFERSIZE);
    memset(str, 0, rbuf*BUFFERSIZE);
    char *str_p = str;
    int i;
    for(i = 0; i < rbuf; ++i) {
        int n = read(sock_fd, str, BUFFERSIZE);
        str = str+BUFFERSIZE;
    }
    return str_p;
}

void sendMsgToServer(int sock_fd, char *str) {
    int sbuf = (strlen(str)-1)/BUFFERSIZE + 1;
    int n = write(sock_fd, &sbuf, sizeof(int));
    char *msgToSend = (char*)malloc(sbuf*BUFFERSIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < sbuf; ++i) {
        int n = write(sock_fd, msgToSend, BUFFERSIZE);
        msgToSend += BUFFERSIZE;
    }
}
int airport_valid(char *airplane_name)
{
    char ports[5][50] = {"Tokyo", "Sapporo", "Naha", "Fukuoka", "Osaka"};
    int ctr = 0;
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(airplane_name, ports[i]) == 0)
        {
            ctr = 1;
        }
    }
    return ctr;
}

int fm(int date, int month, int year)
{
    int fmonth, leap;
    if ((year % 100 == 0) && (year % 400 != 0))
        leap = 0;
    else if (year % 4 == 0)
        leap = 1;
    else
        leap = 0;
    fmonth = 3 + (2 - leap) * ((month + 2) / (2 * month)) + (5 * month + month / 9) / 2;
    fmonth = fmonth % 7;
    return fmonth;
}

//function that converts the date to day.
char *day_of_week(int date, int month, int year)
{
    int dayOfWeek;
    int YY = year % 100;
    int century = year / 100;
    char *day = malloc(20);
    printf("\nDate: %d/%d/%d \n", date, month, year);

    dayOfWeek = 1.25 * YY + fm(date, month, year) + date - 2 * (century % 4);

    dayOfWeek = dayOfWeek % 7;
    if (dayOfWeek == 0)
        strcpy(day, "Saturday\n");
    else if (dayOfWeek == 1)
        strcpy(day, "Sunday\n");
    else if (dayOfWeek == 2)
        strcpy(day, "Monday\n");
    else if (dayOfWeek == 3)
        strcpy(day, "Tuesday\n");
    else if (dayOfWeek == 4)
        strcpy(day, "Wednesday\n");
    else if (dayOfWeek == 5)
        strcpy(day, "Thursday\n");
    else if (dayOfWeek == 6)
        strcpy(day, "Friday\n");
    else
        strcpy(day, "Invalid Date");
    return day;
}

int IsLeapYear(int year)
{
    return (((year % 4 == 0) &&
             (year % 100 != 0)) ||
            (year % 400 == 0));
}
// returns true i.e. 1, if given date is valid.
int isValidDate(Date *validDate)
{
    time_t timing = time(NULL);
    struct tm tm = *localtime(&timing);
    if (validDate->yyyy > MAX_YR ||
        validDate->yyyy < (tm.tm_year + 1900)) //if the year that you have enter is less than the minim year or greater than the max year than its a invalid year
        return 0;
    if (validDate->mm < (tm.tm_mon + 1) || validDate->mm > 12) //similarly no of month is basically 12 hence the enter number should not be more than 12 or less than 1
        return 0;
    if (validDate->dd < tm.tm_mday || validDate->dd > 31)
        return 0;
    //Handle feb days in leap year
    if (validDate->mm == 2) // as we know that leap year has 29 days in feb in constrast to normal year which has only 28 days
    {
        if (IsLeapYear(validDate->yyyy))
            return (validDate->dd <= 29); //leap year has 29 days in feb
        else
            return (validDate->dd <= 28);
    }
    if (validDate->mm == 4 || validDate->mm == 6 ||
        validDate->mm == 9 || validDate->mm == 11) // respective month april, June, september and november has 30 days
        return (validDate->dd <= 30);
    return 1;
}
char *Day(Date *getDate)
{

    int status = 0;
    char *day_string = day_of_week(getDate->dd, getDate->mm, getDate->yyyy);
    printf("You journey is on = %s\n", day_string);
    return day_string;
}

// price conversion for economy class
// 75% discount for infants, 50% for childs, and 0% for adults.
float convert_e(int *age, float *price)
{
    float new_price;
    if (*age < 5)
    { new_price = 0.25 * (*price);  }
    else if (*age < 18 && *age >= 5)
    {  new_price = 0.5 * (*price);}
    else
    { new_price = *price;  }
    return new_price;
}
// price conversion for economy class
// 60% discount for infants, 35% for childs, and 0% for adults.
float convert_b(int *age, float *price)
{
    float new_price;
    if (*age < 5)
    { new_price = 0.40 * (*price);}
    else if (*age < 18 && *age >= 5)
    { new_price = 0.65 * (*price);  }
    else
    {  new_price = *price;}
    return new_price;
}

void sendMsgToServer(int sock_fd, char *str) {
    int sbuf = (strlen(str)-1)/BUFFERSIZE + 1;
    int n = write(sock_fd, &sbuf, sizeof(int));
    char *msgToSend = (char*)malloc(sbuf*BUFFERSIZE);
    strcpy(msgToSend, str);
    int i;
    for(i = 0; i < sbuf; ++i) {
        int n = write(sock_fd, msgToSend, BUFFERSIZE);
        msgToSend += BUFFERSIZE;
    }
}
    while(1) {
        msgFromServer = receiveMsgFromServer(sock_fd);
        if(msgFromServer == NULL)
            break;
        printf("%s\n",msgFromServer);
        free(msgFromServer);
    }
    printf("Write end closed by the server.\n");
    shutdown(sock_fd, SHUT_RD);
    printf("Connection closed gracefully.\n");
    return 0;


}
