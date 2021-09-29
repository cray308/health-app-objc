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

    id leading = getAnchor(view, anchors.left);
    id trailing = getAnchor(view, anchors.right);

    activateConstraints((id []){
        createConstraint(getAnchor(topDivider, anchors.left), leading, 0),
        createConstraint(getAnchor(topDivider, anchors.right), trailing, 0),
        createConstraint(getAnchor(topDivider, anchors.top), getAnchor(view, anchors.top), 0),

        createConstraint(getAnchor(titleLabel, anchors.left), leading, 0),
        createConstraint(getAnchor(titleLabel, anchors.right), trailing, 0),
        createConstraint(getAnchor(titleLabel, anchors.top),
                         getAnchor(topDivider, anchors.bottom), 5),
        createConstraint(getAnchor(titleLabel, anchors.height), nil, 40),
        createConstraint(getAnchor(titleLabel, anchors.bottom), getAnchor(view, anchors.bottom), 0)
    }, 8);
    releaseObj(topDivider);
    releaseObj(titleLabel);
    return view;
}
