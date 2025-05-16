1. sample 0 和 sample 1 是演示 mcf 标定 mpi 接口，sample 2 和 sample 3 是演示 mcf 基本通路。
2. sample 2 支持 IMX347/os04a10 sensor, 使用 os04a10 时，由于 sensor 是主模式，不能保证采集 2 路数据时间完全同步，所以 sample 效果可能不正确，故推荐使用从模式的 IMX347 sensor。
3. 如果使用的分光棱镜模组镜头有误差，可以使用 mipi crop 来对齐 2 个 sensor 视场。
4. 在 2 路 sensor 有视差的场景时，需要使用标定接口标定，标定接口和标定结果使用，请参考《黑白彩色双路融合 开发参考》 和 《黑白彩色双路融合调试指南》。
5. sample 3 演示开启同步时，白天和夜晚切换；sample 4 演示关闭同步时，白天和晚上切换。
6. sample 5 演示 mcf 在线标定后启动 mcf + venc 的业务。
7. 不同分辨率融合，sample 2~5 可采用 黑白路 os08a20 sensor + 彩色路 os04a10 sensor 的组合启动 mcf 业务。
