//
//  AreaChartRenderer.swift
//  HealthApp
//
//  Created by Christopher Ray on 3/31/21.
//

import UIKit

@objc open class AreaChartFormatter: NSObject, FillFormatter {
    private(set) var boundaryDataSet: LineChartDataSet

    @objc public init(boundaryDataSet: LineChartDataSet) {
        self.boundaryDataSet = boundaryDataSet
    }

    open func getFillLinePosition(dataSet: LineChartDataSetProtocol,
                                  dataProvider: LineChartDataProvider) -> CGFloat {
        return 0
    }
}

open class CustomLineChartRenderer: LineChartRenderer {
    override open func drawLinearFill(context: CGContext, dataSet: LineChartDataSetProtocol,
                                      trans: Transformer, bounds: XBounds) {
        guard let dataProvider = dataProvider,
              let areaChartFormatter = dataSet.fillFormatter as? AreaChartFormatter else {
            return
        }

        let filled = generateFilledPath(
            dataSet: dataSet, fillMin: dataSet.fillFormatter?.getFillLinePosition(
                dataSet: dataSet, dataProvider: dataProvider) ?? 0.0,
            boundaryDataSet: areaChartFormatter.boundaryDataSet,
            bounds: bounds, matrix: trans.valueToPixelMatrix)

        if let fill = dataSet.fill {
            drawFilledPath(context: context, path: filled, fill: fill, fillAlpha: dataSet.fillAlpha)
        } else {
            drawFilledPath(context: context, path: filled,
                           fillColor: dataSet.fillColor, fillAlpha: dataSet.fillAlpha)
        }
    }

    fileprivate func generateFilledPath(dataSet: LineChartDataSetProtocol, fillMin: CGFloat,
                                        boundaryDataSet: LineChartDataSetProtocol,
                                        bounds: XBounds, matrix: CGAffineTransform) -> CGPath {
        let phaseY = animator.phaseY
        let isDrawSteppedEnabled = dataSet.mode == .stepped
        let matrix = matrix
        let filled = CGMutablePath()

        if let entry = dataSet.entryForIndex(bounds.min) {
            if let boundaryEntry = boundaryDataSet.entryForIndex(bounds.min) {
                filled.move(to: CGPoint(x: CGFloat(entry.x), y: CGFloat(boundaryEntry.y * phaseY)),
                            transform: matrix)
            } else {
                filled.move(to: CGPoint(x: CGFloat(entry.x), y: fillMin), transform: matrix)
            }
            filled.addLine(to: CGPoint(x: CGFloat(entry.x), y: CGFloat(entry.y * phaseY)),
                           transform: matrix)
        }

        for xVal in stride(from: (bounds.min + 1), through: bounds.range + bounds.min, by: 1) {
            guard let entryCurr = dataSet.entryForIndex(xVal) else { continue }

            if isDrawSteppedEnabled, let entryPrev = dataSet.entryForIndex(xVal - 1) {
                filled.addLine(
                    to: CGPoint(x: CGFloat(entryCurr.x), y: CGFloat(entryPrev.y * phaseY)),
                    transform: matrix)
            }
            filled.addLine(to: CGPoint(x: CGFloat(entryCurr.x), y: CGFloat(entryCurr.y * phaseY)),
                           transform: matrix)
        }

        if let entry = dataSet.entryForIndex(bounds.range + bounds.min) {
            if let boundaryEntry = boundaryDataSet.entryForIndex(bounds.range + bounds.min) {
                filled.addLine(
                    to: CGPoint(x: CGFloat(entry.x), y: CGFloat(boundaryEntry.y * phaseY)),
                    transform: matrix)
            } else {
                filled.addLine(to: CGPoint(x: CGFloat(entry.x), y: fillMin), transform: matrix)
            }
        }

        for xVal in stride(from: (bounds.min + 1),
                           through: bounds.range + bounds.min, by: 1).reversed() {
            guard let entryCurr = boundaryDataSet.entryForIndex(xVal) else { continue }

            if isDrawSteppedEnabled, let entryPrev = boundaryDataSet.entryForIndex(xVal - 1) {
                filled.addLine(
                    to: CGPoint(x: CGFloat(entryCurr.x), y: CGFloat(entryPrev.y * phaseY)),
                    transform: matrix)
            }
            filled.addLine(to: CGPoint(x: CGFloat(entryCurr.x), y: CGFloat(entryCurr.y * phaseY)),
                           transform: matrix)
        }

        filled.closeSubpath()
        return filled
    }
}
