//
//  ChartDataEntry.swift
//  Charts
//
//  Copyright 2015 Daniel Cohen Gindi & Philipp Jahoda
//  A port of MPAndroidChart for iOS
//  Licensed under Apache License 2.0
//
//  https://github.com/danielgindi/Charts
//

import Foundation

open class ChartDataEntry: ChartDataEntryBase, NSCopying
{
    /// the x value
    @objc open var x = 0.0
    
    public required init()
    {
        super.init()
    }
    
    /// An Entry represents one single entry in the chart.
    ///
    /// - Parameters:
    ///   - x: the x value
    ///   - y: the y value (the actual value of the entry)
    @objc public init(x: Double, y: Double)
    {
        super.init(y: y)
        self.x = x
    }
    
    /// An Entry represents one single entry in the chart.
    ///
    /// - Parameters:
    ///   - x: the x value
    ///   - y: the y value (the actual value of the entry)
    ///   - icon: icon image
    @objc public convenience init(x: Double, y: Double, icon: NSUIImage?)
    {
        self.init(x: x, y: y)
        self.icon = icon
    }
        
    // MARK: NSObject
    
    open override var description: String
    {
        return "ChartDataEntry, x: \(x), y \(y)"
    }
    
    // MARK: NSCopying
    
    open func copy(with zone: NSZone? = nil) -> Any
    {
        let copy = type(of: self).init()
        
        copy.x = x
        copy.y = y
        return copy
    }
}

// MARK: Equatable
extension ChartDataEntry/*: Equatable*/ {
    open override func isEqual(_ object: Any?) -> Bool {
        guard let object = object as? ChartDataEntry else { return false }

        if self === object
        {
            return true
        }

        return y == object.y
            && x == object.x
    }
}
