# Using the Optimizely Go SDK C Wrapper in Nginx

## Introduction

This code demonstrates how to use the [Optimizely Go SDK C Wrapper](https://github.com/optimizely/go-sdk-c-wrapper) in Nginx.

I used the code from [Perusio's nginx-hello-world-module](https://github.com/perusio/nginx-hello-world-module). He in turn
used code from from [Dominic's nginx module guide](http://dominicfallows.com/2011/02/20/hello-world-nginx-module-3/).

This code is not the prettiest and is intended as a demonstration only.

## Installation

   1. First build and install Nginx as usual with `make` and `make install`.

   2. Make sure the [Optimizely C SDK](https://github.com/optimizely/go-sdk-c-wrapper) is built and the library (.so) is
      copied to this directory for whatever platform you are working on.

   3. Adjust the paths in the `Makefile` to Nginx source. Then do `make config build install` in this
      directory. See the `Makefile` for additional supported targets when the source code is updated.

   4. Configure the module. There's only one directive `hello_world` that is supported in the **location** context only.

      When loading dynamically first add the following to the Nginx config file.

          load_module modules/nginx_hello_world_module.so

      Then in the listener section invoke the module in a specific location
          
          location = /test {
             
             hello_world;
          
          }

      Now doing something like:
          
          curl -i http://example.com/test
          
      should return the **hello world** string as the response body.

   5. Restart Nginx - `make restart`.

## Additional details

### /proc/cpu

```
[ec2-user@ip-172-31-19-13 ~]$ cat /proc/cpuinfo
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 63
model name      : Intel(R) Xeon(R) CPU E5-2676 v3 @ 2.40GHz
stepping        : 2
microcode       : 0x43
cpu MHz         : 2400.112
cache size      : 30720 KB
physical id     : 0
siblings        : 1
core id         : 0
cpu cores       : 1
apicid          : 0
initial apicid  : 0
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ht syscall nx rdtscp lm constant_tsc rep_good nopl xtopology cpuid pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand hypervisor lahf_lm abm cpuid_fault invpcid_single pti fsgsbase bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bugs            : cpu_meltdown spectre_v1 spectre_v2 spec_store_bypass l1tf mds swapgs
bogomips        : 4800.6
clflush size    : 64
cache_alignment : 64
address sizes   : 46 bits physical, 48 bits virtual
power management:
```


