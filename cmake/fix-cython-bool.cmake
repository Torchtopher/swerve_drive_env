file(READ "${SOURCE}" FILE_CONTENTS)
string(REPLACE "bool " "bint " FILE_CONTENTS "${FILE_CONTENTS}")
string(REPLACE "bool*" "bint*" FILE_CONTENTS "${FILE_CONTENTS}")
file(WRITE "${TARGET}" "${FILE_CONTENTS}")