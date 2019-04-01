package test;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.CountDownLatch;

/**
 * java多线程下载
 * 主要原理基于http的分段下载，可以指定获取http请求的index
 * 写入文件使用的是raf，可以指定index，随机向文件中写入数据
 * index的单位都是字节数
 *
 * 本案对不同线程进行了计数：
 * 2个线程，总用时：20980 ms
 * 10个线程，总用时：23116 ms
 *
 * and WTF...
 *
 * Created by chenwen.wcw on 2019/3/26.
 */
public class MultiDownload {

    public static final String TEST_URL = "";
    public static final String FILE_NAME = "";
    public static final int THREAD_NUM = 10;

    public static void main(String[] args) throws Exception {
        // 计时
        long start = System.currentTimeMillis();
        CountDownLatch latch = new CountDownLatch(THREAD_NUM);

        URL url = new URL(TEST_URL);
        HttpURLConnection conn = (HttpURLConnection)url.openConnection();
        if (conn.getResponseCode() == 200) {
            // get file size
            int fileSize = conn.getContentLength();
            File file = new File(FILE_NAME);

            RandomAccessFile raf = new RandomAccessFile(file, "rw");
            // 10 threads
            int partSize = fileSize % THREAD_NUM == 0 ? fileSize / THREAD_NUM : fileSize / THREAD_NUM + 1;

            for (int i = 0; i < THREAD_NUM; i++) {
                if (i == THREAD_NUM - 1) {
                    DownloadBlock block = new DownloadBlock(i * partSize, 0, latch);
                    block.start();
                    break;
                }
                DownloadBlock block = new DownloadBlock(i * partSize, (i + 1) * partSize, latch);
                block.start();

            }

        }

        // 等待所有线程完成后，计时
        try {
          latch.await();
        } catch (InterruptedException e) {
            System.out.println(e.getMessage());
        }

        long end = System.currentTimeMillis();
        long used = end - start;
        System.out.println(THREAD_NUM + "个线程，总用时：" + used);
    }

    static class DownloadBlock extends Thread {

        private int startIndex;
        private int endIndex;
        private CountDownLatch latch;

        public DownloadBlock(int startIndex, int endIndex, CountDownLatch latch) {
            this.startIndex = startIndex;
            this.endIndex = endIndex;
            this.latch = latch;
        }

        @Override
        public void run() {
            RandomAccessFile raf = null;
            InputStream is = null;
            try {
                URL url = new URL(TEST_URL);
                HttpURLConnection conn = (HttpURLConnection)url.openConnection();
                conn.setConnectTimeout(5000);
                conn.setRequestMethod("GET");
                // http支持分段下载的关键，设置分段获取数据
                String rangeValue = "bytes=" + startIndex + "-" + (endIndex == 0 ? "" : endIndex);
                System.out.println(Thread.currentThread().getName() + ": I download " + rangeValue);
                conn.setRequestProperty("Range", rangeValue);
                is = conn.getInputStream();

                // 分段写入数据
                raf = new RandomAccessFile(FILE_NAME, "rw");
                // 移动指针到合适的位置
                raf.seek(startIndex);
                int len;
                byte[] buffer = new byte[1024];
                while ((len = is.read(buffer)) != -1) {
                    raf.write(buffer, 0, len);
                }

                System.out.println(Thread.currentThread().getName() + " download completed!");
            } catch (Exception e) {
                System.out.println(Thread.currentThread().getName() + " download block failed !");
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                        System.out.println("close is failed !");
                    }
                }

                if (raf != null) {
                    try {
                        raf.close();
                    } catch (IOException e) {
                        System.out.println("close raf failed !");
                    }
                }

                latch.countDown();
            }
        }
    }

}
