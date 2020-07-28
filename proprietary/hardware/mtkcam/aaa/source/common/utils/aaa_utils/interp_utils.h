#pragma once

template <class TX, class TY>
TY interpLine(TX x1, TY y1, TX x2, TY y2, TX x);

template <class TX, class TY>
TY interpTable(int n, TX *xNode, TY *yNode, TX x);

