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
