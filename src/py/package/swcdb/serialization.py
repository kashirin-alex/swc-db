#


def decode_vi64(bytes_value, at=0):
    """
    Decode Variable Length 64-bit Integer

    Parameters
    ----------
    bytes_value : bytes
        Iterable bytes
    at: int
        the current bytes position

    Returns
    -------
    tuple
        (value, at).

    Example
    --------
    value, at = decode_vi64(bytes_value)
    """

    value = 0
    shift = 0
    for i in bytes_value[at:]:
        value |= (i & 0x7f) << shift
        at += 1
        shift += 7
        if not (i & 0x80):
            break
    return value, at
    #


CELL_HAVE_REVISION  =  0x10
def decode_counter(bytes_value):
    """
    Decode Cell value Counter Serialization

    Parameters
    ----------
    bytes_value : bytes
        Iterable bytes

    Returns
    -------
    tuple
        (value, operator, timestamp).

    Example
    --------
    count, op, ts = counter(cell.v)
    """

    value, at = decode_vi64(bytes_value)
    op = bytes_value[at]
    at += 1
    if op & CELL_HAVE_REVISION:
        ts, at = decode_vi64(bytes_value, at)
    else:
        ts = None
    return value, op, ts
    #
