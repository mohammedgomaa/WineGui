FROM debian:buster-slim
MAINTAINER Melroy van den Berg, melroy@melroy.org

RUN apt-get update
RUN apt-get install -y build-essential cmake libgtkmm-3.0-dev pkg-config                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
RUN apt-get install -y ninja-build doxygen graphviz
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl netbase wget gnupg dirmngr bzr git mercurial openssh-client subversion procps