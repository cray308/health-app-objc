//
//  ChartDataEntryBase.swift
//  Charts
//
//  Copyright 2015 Daniel Cohen Gindi & Philipp Jahoda
//  A port of MPAndroidChart for iOS
//  Licensed under Apache License 2.0
//
//  https://github.com/danielgindi/Charts
//

import Foundation

open class ChartDataEntryBase: NSObject
{
    /// the y value
    @objc open var y = 0.0
    
    /// optional icon image
    @objc open var icon: NSUIImage?
    
    public override required init()
    {
        super.init()
    }
    
    /// An Entry represents one single entry in the chart.
    ///
    /// - Parameters:
    ///   - y: the y value (the actual value of the entry)
    @objc public init(y: Double)
    {
        super.init()
        
        self.y = y
    }
    
    /// - Parameters:
    ///   - y: the y value (the actual value of the entry)
    ///   - icon: icon image
    @objc public convenience init(y: Double, icon: NSUIImage?)
    {
        self.init(y: y)

        self.icon = icon
    }

    // MARK: NSObject
    
    open override var description: String
    {
        return "ChartDataEntryBase, y \(y)"
    }
}

// MARK: Equatable
extension ChartDataEntryBase/*: Equatable*/ {
    open override func isEqual(_ object: Any?) -> Bool {
        guard let object = object as? ChartDataEntryBase else { return false }

        if self === object
        {
            return true
        }

        return y == object.y
    }
}
