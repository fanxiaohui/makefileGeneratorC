../makeheaders $1
extension=".h"
basename=$(basename $1 .c)
echo "#ifndef $basename
#define $basename
$(cat $basename$extension)
#endif" > $basename$extension
