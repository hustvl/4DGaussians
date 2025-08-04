# Base image with CUDA and PyTorch
FROM nvidia/cuda:11.6.2-cudnn8-runtime-ubuntu20.04

ARG DEBIAN_FRONTEND=noninteractive

# System setup
RUN apt-get update && apt-get install -y \
    git wget curl ffmpeg libsm6 libxext6 build-essential cmake python3.7 python3.7-venv python3.7-dev \
    && rm -rf /var/lib/apt/lists/*

# Set up Python environment
RUN ln -s /usr/bin/python3.7 /usr/bin/python && \
    curl -sS https://bootstrap.pypa.io/get-pip.py | python

# Copy code
WORKDIR /app
COPY . /app
RUN git submodule update --init --recursive

# Install Python dependencies
RUN pip install --upgrade pip && \
    pip install -r requirements.txt && \
    pip install -e submodules/depth-diff-gaussian-rasterization && \
    pip install -e submodules/simple-knn && \
    pip install torch==1.13.1+cu116 torchvision torchaudio --extra-index-url https://download.pytorch.org/whl/cu116

RUN pip install fastapi uvicorn pydantic


# Expose port for internal tools/viewers
EXPOSE 6017
EXPOSE 8080

CMD ["uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8080"]

