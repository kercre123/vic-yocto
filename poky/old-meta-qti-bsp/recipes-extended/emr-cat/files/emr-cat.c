/**
 * File: main.cpp
 *
 * Author: daniel
 * Created: 3/08/2018
 *
 * Description: main entry point for emr-cat
 *
 * EMR structure definition taken from <victor>/robot/include/anki/cozmo/shared/factory/emr.h
 *
 * Copyright: Anki, Inc. 2018
 *
 **/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

static const char* kEMRFile = "/dev/block/bootdevice/by-name/emr";

static const char* USAGE_FMT = \
"%s <KEY TO READ>\n" \
"\n" \
"Where <KEY TO READ> is one of:\n" \
"h: Print this help text instead of reading EMR\n" \
"e: ESN\n" \
"v: HW_FER\n" \
"m: MODEL\n" \
"l: LOT_CODE\n" \
"r: PLAYPEN_READY_FLAG\n" \
"p: PLAYPEN_PASSED_FLAG\n" \
"o: PACKED_OUT_FLAG\n" \
"d: PACKED_OUT_DATE\n" \
"a: Entire EMR structure\n";

#define RSLT_FORMAT "%08x"

typedef struct {
  uint32_t ESN;
  uint32_t HW_VER;
  uint32_t MODEL;
  uint32_t LOT_CODE;
  uint32_t PLAYPEN_READY_FLAG; //fixture approves all previous testing. OK to run playpen
  uint32_t PLAYPEN_PASSED_FLAG;
  uint32_t PACKED_OUT_FLAG;
  uint32_t PACKED_OUT_DATE; //Unix time?
  uint32_t reserved[48];
  uint32_t playpen[8];
  uint32_t fixture[192];
} EMR;

int main(int argc, char *argv[]) {
  EMR record;
  int fd = -1;
  ssize_t rdrslt = 0;
  uint32_t result = 0;

  if (argc != 2) {
    fprintf(stderr, USAGE_FMT, argv[0]);
    return 1;
  }

  fd = open(kEMRFile, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Error opening EMR \"%s\": %d\n", kEMRFile, errno);
    fprintf(stdout, RSLT_FORMAT, 0); // Need to echo something so we don't break things down the line
    return fd; // Consumers should check return code if default return isn't good enough
  }

  rdrslt = read(fd, &record, sizeof(EMR));
  if (rdrslt != sizeof(EMR)) {
    fprintf(stderr, "Error reading EMR, expected %d bytes, got %d\n", sizeof(EMR), rdrslt);
    fprintf(stdout, RSLT_FORMAT, 0); // Need to echo something so we don't break things down the line
    return -1; // Consumers should check return code if default return isn't good enough
  }

  switch(argv[1][0]) {
    case 'h':
      printf(USAGE_FMT, argv[0]);
      return 0;
    case 'e':
      result = record.ESN;
      break;
    case 'v':
      result = record.HW_VER;
      break;
    case 'm':
      result = record.MODEL;
      break;
    case 'l':
      result = record.LOT_CODE;
      break;
    case 'r':
      result = record.PLAYPEN_READY_FLAG;
      break;
    case 'p':
      result = record.PLAYPEN_PASSED_FLAG;
      break;
    case 'o':
      result = record.PACKED_OUT_FLAG;
      break;
    case 'd':
      result = record.PACKED_OUT_DATE;
      break;
    case 'a':
      {
        int i;
        uint32_t* record_as_u32arr = &(record.ESN);
        for (i=0; i<(sizeof(EMR)/sizeof(uint32_t)); ++i) {
          fprintf(stdout, RSLT_FORMAT "\n", record_as_u32arr[i]);
        }
        return 0;
      }
    default:
      fprintf(stderr, "Unknown EMR key '%c'\n", argv[1][0]);
      return -1;
  }

  fprintf(stdout, RSLT_FORMAT, result);
  return 0;
}
