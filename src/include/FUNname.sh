grep '^[a-z]' protodef.h | sed -e 's/^[a-zA-Z]* [\*]*//' -e 's/(.*//' | sort
