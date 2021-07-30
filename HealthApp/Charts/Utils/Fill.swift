//
//  Fill.swift
//  Charts
//
//  Copyright 2015 Daniel Cohen Gindi & Philipp Jahoda
//  A port of MPAndroidChart for iOS
//  Licensed under Apache License 2.0
//
//  https://github.com/danielgindi/Charts
//

import Foundation
import CoreGraphics

@objc(ChartFill)
public protocol Fill
{

    /// Draws the provided path in filled mode with the provided area
    @objc func fillPath(context: CGContext, rect: CGRect)
}

@objc(ChartLinearGradientFill)
public class LinearGradientFill: NSObject, Fill
{

    @objc public let gradient: CGGradient
    @objc public let angle: CGFloat

    @objc public init(gradient: CGGradient, angle: CGFloat = 0)
    {
        self.gradient = gradient
        self.angle = angle
        super.init()
    }

    public func fillPath(context: CGContext, rect: CGRect)
    {
        context.saveGState()
        defer { context.restoreGState() }

        let radians = (360.0 - angle).DEG2RAD
        let centerPoint = CGPoint(x: rect.midX, y: rect.midY)
        let xAngleDelta = cos(radians) * rect.width / 2.0
        let yAngleDelta = sin(radians) * rect.height / 2.0
        let startPoint = CGPoint(
            x: centerPoint.x - xAngleDelta,
            y: centerPoint.y - yAngleDelta
        )
        let endPoint = CGPoint(
            x: centerPoint.x + xAngleDelta,
            y: centerPoint.y + yAngleDelta
        )

        context.clip()
        context.drawLinearGradient(
            gradient,
            start: startPoint,
            end: endPoint,
            options: [.drawsAfterEndLocation, .drawsBeforeStartLocation]
        )
    }
}
