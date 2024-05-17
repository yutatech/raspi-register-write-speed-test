#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstdint>
#include <chrono>
#include <thread>
#include <bitset>


const uint32_t GPIO_BASE = 0xFE200000;
const uint32_t GPIO_SIZE = 0x000000F4;
typedef struct
{
  volatile uint32_t GPFSEL0;               // 0x00
  volatile uint32_t GPFSEL1;               // 0x04
  volatile uint32_t GPFSEL2;               // 0x08
  volatile uint32_t GPFSEL3;               // 0x0c
  volatile uint32_t GPFSEL4;               // 0x10
  volatile uint32_t GPFSEL5;               // 0x14
  volatile uint32_t RESERVED_1;            // 0x18
  volatile uint32_t GPSET0;                // 0x1c
  volatile uint32_t GPSET1;                // 0x20
  volatile uint32_t RESERVED_2;            // 0x24
  volatile uint32_t GPCLR0;                // 0x28
  volatile uint32_t GPCLR1;                // 0x2c
  volatile uint32_t RESERVED_3;            // 0x30
  volatile uint32_t GPLEV0;                // 0x34
  volatile uint32_t GPLEV1;                // 0x38
  volatile uint32_t RESERVED_4;            // 0x3c
  volatile uint32_t GPEDS0;                // 0x40
  volatile uint32_t GPEDS1;                // 0x44
  volatile uint32_t RESERVED_5;            // 0x48
  volatile uint32_t GPREN0;                // 0x4c
  volatile uint32_t GPREN1;                // 0x50
  volatile uint32_t RESERVED_6;            // 0x54
  volatile uint32_t GPFEN0;                // 0x58
  volatile uint32_t GPFEN1;                // 0x5c
  volatile uint32_t RESERVED_7;            // 0x60
  volatile uint32_t GPHEN0;                // 0x64
  volatile uint32_t GPHEN1;                // 0x68
  volatile uint32_t RESERVED_8;            // 0x6c
  volatile uint32_t GPLEN0;                // 0x70
  volatile uint32_t GPLEN1;                // 0x74
  volatile uint32_t RESERVED_9;            // 0x78
  volatile uint32_t GPAREN0;               // 0x7c
  volatile uint32_t GPAREN1;               // 0x80
  volatile uint32_t RESERVED_10;           // 0x84
  volatile uint32_t GPAFEN0;               // 0x88
  volatile uint32_t GPAFEN1;               // 0x8c
  volatile uint32_t RESERVED_11[21];       // 0x90~0xe0
  volatile uint32_t PUP_PDN_CNTRL_REG0;    // 0xe4
  volatile uint32_t PUP_PDN_CNTRL_REG1;    // 0xe8
  volatile uint32_t PUP_PDN_CNTRL_REG2;    // 0xec
  volatile uint32_t PUP_PDN_CNTRL_REG3;    // 0xf0
} GPIO_Typedef;

struct GPSET0 {
  uint32_t SET0 : 1;
  uint32_t SET1 : 1;
  uint32_t SET2 : 1;
  uint32_t SET3 : 1;
  uint32_t SET4 : 1;
  uint32_t SET5 : 1;
  uint32_t SET6 : 1;
  uint32_t SET7 : 1;
  uint32_t SET8 : 1;
  uint32_t SET9 : 1;
  uint32_t SET10 : 1;
  uint32_t SET11 : 1;
  uint32_t SET12 : 1;
  uint32_t SET13 : 1;
  uint32_t SET14 : 1;
  uint32_t SET15 : 1;
  uint32_t SET16 : 1;
  uint32_t SET17 : 1;
  uint32_t SET18 : 1;
  uint32_t SET19 : 1;
  uint32_t SET20 : 1;
  uint32_t SET21 : 1;
  uint32_t SET22 : 1;
  uint32_t SET23 : 1;
  uint32_t SET24 : 1;
  uint32_t SET25 : 1;
  uint32_t SET26 : 1;
  uint32_t SET27 : 1;
  uint32_t SET28 : 1;
  uint32_t SET29 : 1;
  uint32_t SET30 : 1;
  uint32_t SET31 : 1;
};

volatile GPSET0 *gpset0;
volatile GPSET0 *gpclr0;

enum class GPIO_Function : uint8_t
{
  INPUT  = 0b000,
  OUTPUT = 0b001,
  ALT0   = 0b100,
  ALT1   = 0b101,
  ALT2   = 0b110,
  ALT3   = 0b111,
  ALT4   = 0b011,
  ALT5   = 0b010,
};

int main() {
  int fd;
	if ((fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
    std::cout << "Can't open /dev/mem" << std::endl;
    return -1;
  }

  GPIO_Typedef* REG_GPIO;
	REG_GPIO = (GPIO_Typedef*) mmap(NULL, GPIO_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
  
  uint8_t pin = 12;

  uint8_t bit_mode = static_cast<uint8_t>(GPIO_Function::OUTPUT);
  REG_GPIO->GPFSEL1 &= ~(0b111 << (pin-10)*3);
  REG_GPIO->GPFSEL1 |= bit_mode << (pin-10)*3;

  uint8_t bit_pull_up_down = 0b00;
  REG_GPIO->GPFSEL5 &= ~(0b11 << pin*2);
  REG_GPIO->GPFSEL5 |= bit_pull_up_down << pin*2;

  gpset0 = reinterpret_cast<volatile GPSET0*>(&(REG_GPIO->GPSET0));
  gpclr0 = reinterpret_cast<volatile GPSET0*>(&(REG_GPIO->GPCLR0));


  REG_GPIO->GPFSEL1 |= bit_mode << (pin-10)*3;

  using namespace std::literals::chrono_literals;
  std::chrono::system_clock::time_point start, end;

  start = std::chrono::system_clock::now();
  for(int a=0; a<1000000; a++) {
    REG_GPIO->GPSET0 |= 0b1<<pin;
    REG_GPIO->GPCLR0 |= 0b1<<pin;
  }
  end = std::chrono::system_clock::now();
  uint64_t time1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  start = std::chrono::system_clock::now();
  for(int a=0; a<1000000; a++) {
    gpset0->SET12 = 1;
    gpclr0->SET12 = 1;
  }
  end = std::chrono::system_clock::now();
  uint64_t time2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  std::cout << "bitwise operation  " << time1 << "[us]" << std::endl;
  std::cout << "bit field  " << time2 << "[us]" << std::endl;
  std::cout << "bit field is faster than bitwise operation  " << time1 - time2 << "[us]" << std::endl;
}