FROM centos:7.5.1804
LABEL author="MCL <c.y.lam@ljmu.ac.uk>"

# install dnf
RUN yum install -y epel-release
RUN yum install -y dnf

# Remove the epel-release after sorting out dnf
RUN yum remove -y epel-release

# Add non-root user "data" inside the container
RUN dnf install -y sudo \
  && groupadd -g 600 web \
  && adduser -u 501 -g 600 data \
  && echo "user ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/user \
  && chmod 0440 /etc/sudoers.d/user

# Add the working directory and temporary download space
ENV WORKSPACE=/space/home/dev/src
WORKDIR $WORKSPACE
ENV DDIR=$WORKSPACE/downloads

# Update docker image
RUN yum -y upgrade

# Install tools
RUN yum install -y gcc gcc-c++ kernel-devel wget git curl make tcsh \
  && yum install -y python-devel libxslt-devel libffi-devel openssl-devel

# Download and install gsl
RUN wget -P $DDIR/ https://ftp.gnu.org/gnu/gsl/gsl-2.5.tar.gz \
  && mkdir $DDIR/gsl \
  && tar -xf $DDIR/gsl-2.5.tar.gz -C $DDIR/gsl --strip 1 \
  && cd $DDIR/gsl/ \
  && ./configure --prefix=/usr \
  && make \
  && make install

# Download cfitsio
RUN wget -P $DDIR/ https://src.fedoraproject.org/lookaside/pkgs/cfitsio/cfitsio3310.tar.gz/75b6411751c7f308d45b281b7beb92d6/cfitsio3310.tar.gz \
  && mkdir $WORKSPACE/cfitsio3310 \
  && tar -xf $DDIR/cfitsio3310.tar.gz -C $WORKSPACE/cfitsio3310 --strip 1 \
  && cd $WORKSPACE/cfitsio3310 \
  && ./configure --prefix=/usr \
  && make \
  && make install

# Follow instructions from https://centos.pkgs.org/7/centos-sclo-rh-testing/python27-tkinter-2.7.8-16.el7.x86_64.rpm.html
RUN yum install -y tkinter
RUN yum install -y centos-release-scl-rh \
  && yum -y --enablerepo=centos-sclo-rh-testing install python27-tkinter

# Install Python package manager pip 
RUN curl "https://bootstrap.pypa.io/get-pip.py" -o "$DDIR/get-pip.py" \
  && python $DDIR/get-pip.py

# Install Python packages
# backports.functools-lru-cache is necessary to solve the "missing site-package/six" problem
RUN pip install backports.functools-lru-cache \
  && pip install numpy \
  && pip install pyfits \
  && pip install matplotlib

# Copy over and make the SPRAT L2 pipeline
ADD sprat_l2_pipeline $WORKSPACE/sprat_l2_pipeline
RUN tcsh -c "source $WORKSPACE/sprat_l2_pipeline/scripts/L2_setup && cd $WORKSPACE/sprat_l2_pipeline/src && make all"
RUN cat $WORKSPACE/sprat_l2_pipeline/scripts/L2_setup >> /etc/csh.cshrc

# Create folders for mounting to the host system
RUN mkdir $WORKSPACE/output_test
RUN mkdir $WORKSPACE/sprat

RUN chown -R data:web $WORKSPACE


