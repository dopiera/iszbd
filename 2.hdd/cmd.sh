fio --name=randwrite4k \
    --filename=/dev/sdX \
    --rw=randwrite \
    --bs=QWE \
    --direct=1 \
    --ioengine=libaio \
    --iodepth=XYZ \
    --rate_iops=ABC \
    --rate_process=poisson \
    --time_based \
    --runtime=60 \
    --numjobs=1

