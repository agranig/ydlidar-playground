#include "CYdLidar.h"
#include <iostream>
#include <string>
#include <signal.h>
#include <memory>
#include <unistd.h>

using namespace ydlidar;

CYdLidar laser;
static sig_atomic_t quit = false;

static void Stop(int signo) {  
    quit = 1;
}  

int main(int argc, char * argv[]) {
    bool showHelp = argc > 1 && !strcmp(argv[1], "--help");
    if (argc < 4 || showHelp) {
        printf("Usage: %s <serial_port> <baudrate> <intensities>\n\n",argv[0]);
        printf("Example:%s /dev/ttyUSB0 115200 0\n\n",argv[0]);
        if (!showHelp)
            return -1;
        else
            return 0;
    }

    const std::string port = std::string(argv[1]);
    const int baud = atoi(argv[2]);
    const int intensities = atoi(argv[3]);

    signal(SIGINT, Stop);
    signal(SIGTERM, Stop);

    laser.setSerialPort(port);
    laser.setSerialBaudrate(baud);
    laser.setIntensities(intensities);

    laser.initialize();

    std::vector<float> ranges;
    std::vector<float> ints;
    int done = 0;
    int missing, lastmissing;
    int nonew = 0;

    while(!quit && !done && nonew < 100) {
        bool hardError;
        LaserScan scan;

        if (laser.doProcessSimple(scan, hardError)) {
            fprintf(stderr,"Scan received: %u ranges/%u intensities\n",(unsigned int)scan.ranges.size(), (unsigned int)scan.intensities.size());
            fprintf(stderr,
                    "Scan config:\n"
                    "  min_angle: %f\n"
                    "  max_angle: %f\n"
                    "  ang_increment: %f\n"
                    "  time_increment: %f\n"
                    "  scan_time: %f\n"
                    "  min_range: %f\n"
                    "  max_range: %f\n"
                    "  range_res: %f\n",
                    scan.config.min_angle * 180 / M_PI,
                    scan.config.max_angle * 180 / M_PI,
                    scan.config.ang_increment * 180 / M_PI,
                    scan.config.time_increment,
                    scan.config.scan_time,
                    scan.config.min_range,
                    scan.config.max_range,
                    scan.config.range_res
            );
            if (!ranges.size()) {
                ranges = scan.ranges;
                ints = scan.intensities;
                missing = lastmissing = ints.size();
            } else {
                int tmpdone = 1;
                missing = ints.size();
                for (unsigned int i = 0; i < ints.size(); ++i) {
                    if (scan.intensities[i] > 0) {
                        ranges[i] = scan.ranges[i];
                        ints[i] = scan.intensities[i];
                    }
                    tmpdone &= (ints[i] > 0);
                    if (ints[i] > 0) {
                        missing--;
                    }
                }
                done = tmpdone;
            }

        }
        fprintf(stderr, "%u of %u points captured\n",
                ints.size() - missing, ints.size());
        if (lastmissing = missing)
            nonew++;
        else
            nonew = 0;
        lastmissing = missing;

        usleep(50*1000);
    }

    laser.turnOff();
    laser.disconnecting();

    for (unsigned int i = 0; i < ranges.size(); ++i) {
        fprintf(stderr, "%.03f ", ranges[i]);
    }
    fprintf(stderr, "\n");

    return 0;
}
