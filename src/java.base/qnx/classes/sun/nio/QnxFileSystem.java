package sun.nio.fs;

import java.nio.file.*;
import java.io.IOException;
import java.util.*;

/**
 * Qnx implementation of FileSystem
 */

class QnxFileSystem extends UnixFileSystem {

    QnxFileSystem(UnixFileSystemProvider provider, String dir) {
        super(provider, dir);
    }

    @Override
    public WatchService newWatchService()
        throws IOException
    {
        // use polling implementation until we implement a QNX one
        return new PollingWatchService();
    }

    // lazy initialization of the list of supported attribute views
    private static class SupportedFileFileAttributeViewsHolder {
        static final Set<String> supportedFileAttributeViews =
            supportedFileAttributeViews();
        private static Set<String> supportedFileAttributeViews() {
            Set<String> result = new HashSet<String>();
            result.addAll(standardFileAttributeViews());
            return Collections.unmodifiableSet(result);
        }
    }

    @Override
    public Set<String> supportedFileAttributeViews() {
        return SupportedFileFileAttributeViewsHolder.supportedFileAttributeViews;
    }

    /**
     * Returns object to iterate over mount entries
     */
    @Override
    Iterable<UnixMountEntry> getMountEntries() {
        ArrayList<UnixMountEntry> entries = new ArrayList<UnixMountEntry>();
        try {
            long iter = QnxNativeDispatcher.getfsstat();
            try {
                for (;;) {
                    UnixMountEntry entry = new UnixMountEntry();
                    int res = QnxNativeDispatcher.fsstatEntry(iter, entry);
                    if (res < 0)
                        break;
                    entries.add(entry);
                }
            } finally {
                QnxNativeDispatcher.endfsstat(iter);
            }

        } catch (UnixException x) {
            // nothing we can do
        }
        return entries;
    }



    @Override
    FileStore getFileStore(UnixMountEntry entry) throws IOException {
        return new QnxFileStore(this, entry);
    }
}

