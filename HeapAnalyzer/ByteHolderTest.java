class ByteHolder {
    byte[] b;
    ByteHolder(int size) {
        b = new byte[size*1024*1024];
    }
}

public class ByteHolderTest {
    public static void main(String[] args) throws Exception {
        ByteHolder bh1 = new ByteHolder(100);
        ByteHolder bh2 = new ByteHolder(200);
        while (true) {
            Thread.sleep(5000);
        }
    }
}
