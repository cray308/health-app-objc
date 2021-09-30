//
//  HistoryChartHelpers.c
//  HealthApp
//
//  Created by Christopher Ray on 8/23/21.
//

#include "HistoryChartHelpers.h"
#include "ViewControllerHelpers.h"

id createChartSeparator(CFStringRef title) {
    id view = createView(nil, false);
    id topDivider = createDivider();
    id titleLabel = createLabel(title, 0, 1);
    setLabelFontWithSize(titleLabel, 20);
    addSubview(view, topDivider);
    addSubview(view, titleLabel);

    pin(topDivider, view, (Padding){0}, EdgeBottom);
    pin(titleLabel, view, (Padding){0}, EdgeTop);
    setHeight(titleLabel, 40);
    pinTopToBottom(titleLabel, topDivider, 5);
    releaseObj(topDivider);
    releaseObj(titleLabel);
    return view;
}
