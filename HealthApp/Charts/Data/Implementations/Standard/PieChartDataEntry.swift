//
//  PieChartDataEntry.swift
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

open class PieChartDataEntry: ChartDataEntry
{
    public required init()
    {
        super.init()
    }
    

    /// - Parameters:
    ///   - value: The value on the y-axis
    @objc public init(value: Double)
    {
        super.init(x: .nan, y: value)
    }

    /// - Parameters:
    ///   - value: The value on the y-axis
    ///   - label: The label for the x-axis
    @objc public convenience init(value: Double, label: String?)
    {
        self.init(value: value)
        self.label = label
    }
    
    /// - Parameters:
    ///   - value: The value on the y-axis
    ///   - label: The label for the x-axis
    ///   - icon: icon image
    @objc public convenience init(value: Double, label: String?, icon: NSUIImage?)
    {
        self.init(value: value)
        self.label = label
        self.icon = icon
    }
    
    /// - Parameters:
    ///   - value: The value on the y-axis
    ///   - icon: icon image
    @objc public convenience init(value: Double, icon: NSUIImage?)
    {
        self.init(value: value)
        self.icon = icon
    }
    
    // MARK: Data property accessors
    
    @objc open var label: String?
    
    @objc open var value: Double
    {
        get { return y }
        set { y = newValue }
    }
        
    // MARK: NSCopying
    
    open override func copy(with zone: NSZone? = nil) -> Any
    {
        let copy = super.copy(with: zone) as! PieChartDataEntry
        copy.label = label
        return copy
    }
}
