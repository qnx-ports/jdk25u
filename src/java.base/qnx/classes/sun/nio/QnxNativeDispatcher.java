package sun.nio.fs;

/**
 * Qnx specific system calls.
 */

class QnxNativeDispatcher extends UnixNativeDispatcher {
    protected QnxNativeDispatcher() { }

   /**
    * struct fsstat_iter *getfsstat();
    */
    static native long getfsstat() throws UnixException;

   /**
    * int fsstatEntry(struct fsstat_iter * iter, UnixMountEntry entry);
    */
    static native int fsstatEntry(long iter, UnixMountEntry entry)
        throws UnixException;

   /**
    * void endfsstat(struct fsstat_iter * iter);
    */
    static native void endfsstat(long iter) throws UnixException;

    // initialize field IDs
    private static native void initIDs();

    static {
         initIDs();
    }
}
