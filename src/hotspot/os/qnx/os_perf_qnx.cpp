#include "memory/allocation.inline.hpp"
#include "memory/resourceArea.hpp"
#include "runtime/os.hpp"
#include "runtime/os_perf.hpp"
#include "runtime/vm_version.hpp"
#include "utilities/globalDefinitions.hpp"


#include <sys/sysctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

class CPUPerformanceInterface::CPUPerformance : public CHeapObj<mtInternal> {
   friend class CPUPerformanceInterface;

  private:

   int cpu_load(int which_logical_cpu, double* cpu_load);
   int context_switch_rate(double* rate);
   int cpu_load_total_process(double* cpu_load);
   int cpu_loads_process(double* pjvmUserLoad, double* pjvmKernelLoad, double* psystemTotalLoad);

   NONCOPYABLE(CPUPerformance);

  public:
    CPUPerformance();
    bool initialize();
    ~CPUPerformance();
};


CPUPerformanceInterface::CPUPerformance::CPUPerformance() {
}

bool CPUPerformanceInterface::CPUPerformance::initialize() {
  return true;
}

CPUPerformanceInterface::CPUPerformance::~CPUPerformance() {
}

int CPUPerformanceInterface::CPUPerformance::cpu_load(int which_logical_cpu, double* cpu_load) {
  return FUNCTIONALITY_NOT_IMPLEMENTED;
}

int CPUPerformanceInterface::CPUPerformance::context_switch_rate(double* rate) {
  return FUNCTIONALITY_NOT_IMPLEMENTED;
}

int CPUPerformanceInterface::CPUPerformance::cpu_load_total_process(double* cpu_load) {
  return FUNCTIONALITY_NOT_IMPLEMENTED;
}

int CPUPerformanceInterface::CPUPerformance::cpu_loads_process(double* pjvmUserLoad, double* pjvmKernelLoad, double* psystemTotalLoad) {
  return FUNCTIONALITY_NOT_IMPLEMENTED;
}

CPUPerformanceInterface::CPUPerformanceInterface() {
  _impl = nullptr;
}

bool CPUPerformanceInterface::initialize() {
  _impl = new CPUPerformanceInterface::CPUPerformance();
  return _impl->initialize();
}

CPUPerformanceInterface::~CPUPerformanceInterface() {
  if (_impl != nullptr) {
    delete _impl;
  }
}

int CPUPerformanceInterface::cpu_load(int which_logical_cpu, double* cpu_load) const {
  return _impl->cpu_load(which_logical_cpu, cpu_load);
}

int CPUPerformanceInterface::cpu_load_total_process(double* cpu_load) const {
  return _impl->cpu_load_total_process(cpu_load);
}

int CPUPerformanceInterface::cpu_loads_process(double* pjvmUserLoad, double* pjvmKernelLoad, double* psystemTotalLoad) const {
  return _impl->cpu_loads_process(pjvmUserLoad, pjvmKernelLoad, psystemTotalLoad);
}

int CPUPerformanceInterface::context_switch_rate(double* rate) const {
  return _impl->context_switch_rate(rate);
}

class SystemProcessInterface::SystemProcesses : public CHeapObj<mtInternal> {
  friend class SystemProcessInterface;
 private:
  SystemProcesses();
  bool initialize();
  NONCOPYABLE(SystemProcesses);
  ~SystemProcesses();

  //information about system processes
  int system_processes(SystemProcess** system_processes, int* no_of_sys_processes) const;
};

SystemProcessInterface::SystemProcesses::SystemProcesses() {
}

bool SystemProcessInterface::SystemProcesses::initialize() {
  return true;
}

SystemProcessInterface::SystemProcesses::~SystemProcesses() {
}

int SystemProcessInterface::SystemProcesses::system_processes(SystemProcess** system_processes, int* no_of_sys_processes) const {
  assert(system_processes != nullptr, "system_processes pointer is null!");
  assert(no_of_sys_processes != nullptr, "system_processes counter pointer is null!");

  return FUNCTIONALITY_NOT_IMPLEMENTED;
}

int SystemProcessInterface::system_processes(SystemProcess** system_procs, int* no_of_sys_processes) const {
  return _impl->system_processes(system_procs, no_of_sys_processes);
}

SystemProcessInterface::SystemProcessInterface() {
  _impl = nullptr;
}

bool SystemProcessInterface::initialize() {
  _impl = new SystemProcessInterface::SystemProcesses();
  return _impl->initialize();
}

SystemProcessInterface::~SystemProcessInterface() {
  if (_impl != nullptr) {
    delete _impl;
 }
}

CPUInformationInterface::CPUInformationInterface() {
  _cpu_info = nullptr;
}

bool CPUInformationInterface::initialize() {
  _cpu_info = new CPUInformation();
  VM_Version::initialize_cpu_information();
  _cpu_info->set_number_of_hardware_threads(VM_Version::number_of_threads());
  _cpu_info->set_number_of_cores(VM_Version::number_of_cores());
  _cpu_info->set_number_of_sockets(VM_Version::number_of_sockets());
  _cpu_info->set_cpu_name(VM_Version::cpu_name());
  _cpu_info->set_cpu_description(VM_Version::cpu_description());
  return true;
}

CPUInformationInterface::~CPUInformationInterface() {
  if (_cpu_info != nullptr) {
    if (_cpu_info->cpu_name() != nullptr) {
      const char* cpu_name = _cpu_info->cpu_name();
      FREE_C_HEAP_ARRAY(char, cpu_name);
      _cpu_info->set_cpu_name(nullptr);
    }
    if (_cpu_info->cpu_description() != nullptr) {
      const char* cpu_desc = _cpu_info->cpu_description();
      FREE_C_HEAP_ARRAY(char, cpu_desc);
      _cpu_info->set_cpu_description(nullptr);
    }
    delete _cpu_info;
  }
}

int CPUInformationInterface::cpu_information(CPUInformation& cpu_info) {
  if (nullptr == _cpu_info) {
    return OS_ERR;
  }

  cpu_info = *_cpu_info; // shallow copy assignment
  return OS_OK;
}

class NetworkPerformanceInterface::NetworkPerformance : public CHeapObj<mtInternal> {
  friend class NetworkPerformanceInterface;
 private:
  NetworkPerformance();
  NONCOPYABLE(NetworkPerformance);
  bool initialize();
  ~NetworkPerformance();
  int network_utilization(NetworkInterface** network_interfaces) const;
};

NetworkPerformanceInterface::NetworkPerformance::NetworkPerformance() {
}

bool NetworkPerformanceInterface::NetworkPerformance::initialize() {
  return true;
}

NetworkPerformanceInterface::NetworkPerformance::~NetworkPerformance() {
}

int NetworkPerformanceInterface::NetworkPerformance::network_utilization(NetworkInterface** network_interfaces) const {
  size_t len;
  int mib[] = {CTL_NET, PF_ROUTE, /* protocol number */ 0, /* address family */ 0, NET_RT_IFLIST, /* NET_RT_FLAGS mask*/ 0};
  if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), nullptr, &len, nullptr, 0) != 0) {
    return OS_ERR;
  }
  uint8_t* buf = NEW_RESOURCE_ARRAY(uint8_t, len);
  if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), buf, &len, nullptr, 0) != 0) {
    return OS_ERR;
  }

  size_t index = 0;
  NetworkInterface* ret = nullptr;
  while (index < len) {
    if_msghdr* msghdr = reinterpret_cast<if_msghdr*>(buf + index);
    index += msghdr->ifm_msglen;

    if (msghdr->ifm_type != RTM_IFINFO) {
      continue;
    }

    sockaddr_dl* sockaddr = reinterpret_cast<sockaddr_dl*>(msghdr + 1);

    // The interface name is not necessarily NUL-terminated
    char name_buf[128];
    size_t name_len = MIN2(sizeof(name_buf) - 1, static_cast<size_t>(sockaddr->sdl_nlen));
    strlcpy(name_buf, sockaddr->sdl_data, name_len + 1);

    uint64_t bytes_in = msghdr->ifm_data.ifi_ibytes;
    uint64_t bytes_out = msghdr->ifm_data.ifi_obytes;

    NetworkInterface* cur = new NetworkInterface(name_buf, bytes_in, bytes_out, ret);
    ret = cur;
  }

  *network_interfaces = ret;

  return OS_OK;
}

NetworkPerformanceInterface::NetworkPerformanceInterface() {
  _impl = nullptr;
}

NetworkPerformanceInterface::~NetworkPerformanceInterface() {
  if (_impl != nullptr) {
    delete _impl;
  }
}

bool NetworkPerformanceInterface::initialize() {
  _impl = new NetworkPerformanceInterface::NetworkPerformance();
  return _impl->initialize();
}

int NetworkPerformanceInterface::network_utilization(NetworkInterface** network_interfaces) const {
  return _impl->network_utilization(network_interfaces);
}
