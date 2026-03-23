package sun.nio.fs;

import java.nio.file.*;
import java.nio.file.attribute.*;
import java.io.IOException;

/**
 * Qnx implementation of FileSystemProvider
 */

public class QnxFileSystemProvider extends UnixFileSystemProvider {
    public QnxFileSystemProvider() {
        super();
    }

    @Override
    QnxFileSystem newFileSystem(String dir) {
        return new QnxFileSystem(this, dir);
    }

    @Override
    QnxFileStore getFileStore(UnixPath path) throws IOException {
        return new QnxFileStore(path);
    }
}

