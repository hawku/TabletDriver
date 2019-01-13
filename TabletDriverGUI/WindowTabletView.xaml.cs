using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace TabletDriverGUI
{

    /// <summary>
    /// Interaction logic for WindowTabletView.xaml
    /// </summary>
    public partial class WindowTabletView : Window
    {
        Configuration config;
        TabletDriver driver;
        TabletRenderer tabletRenderer;
        MultimediaTimer timer;
        Vector lastPosition;
        DateTime lastUpdate;
        DateTime lastInputStartTime;
        double lastPressure;
        bool hadInputLoss;
        double velocity;
        double latency;


        //
        // TabletViewElement
        //
        public class TabletRenderer : UIElement
        {

            public Point[] LastPositionsInput;
            public Point[] LastPositionsOutput;
            public Point[] DrawPositions;
            public Point PositionInput;
            public Point PositionOutput;
            public double PressureInput;
            public double PressureOutput;
            public double ScaleX;
            public double ScaleY;

            public readonly Brush brushInput;
            public readonly Brush brushOutput;
            public readonly Brush brushDraw;

            private readonly Pen penInputLine;
            private readonly Pen penOutputLine;
            private readonly Pen penDrawLine;
            private readonly Pen penInputCircle;
            private readonly Pen penOutputCircle;
            private readonly Pen penInputPressure;
            private readonly Pen penOutputPressure;

            public Rect rectInputPressure;
            public Rect rectOutputPressure;
            public readonly Rect rectInputPressureBorder;
            public readonly Rect rectOutputPressureBorder;


            //
            // Tablet renderer constructor
            //
            public TabletRenderer(Configuration config)
            {
                //
                // Last position arrays
                //
                LastPositionsInput = new Point[config.TabletView.InputTrailLength];
                LastPositionsOutput = new Point[config.TabletView.OutputTrailLength];
                for (int i = 0; i < LastPositionsInput.Length; i++)
                    LastPositionsInput[i] = new Point(-1, -1);
                for (int i = 0; i < LastPositionsOutput.Length; i++)
                    LastPositionsOutput[i] = new Point(-1, -1);


                //
                // Draw position array
                //
                DrawPositions = new Point[config.TabletView.DrawLength];
                for (int i = 0; i < DrawPositions.Length; i++)
                {
                    DrawPositions[i] = new Point(-1, -1);
                }

                // Input/output positions
                PositionInput = new Point(0, 0);
                PositionOutput = new Point(0, 0);

                // Colors
                Color colorInput = Colors.Green;
                try { colorInput = (Color)ColorConverter.ConvertFromString(config.TabletView.InputColor); } catch (Exception) { }
                Color colorOutput = Colors.Red;
                try { colorOutput = (Color)ColorConverter.ConvertFromString(config.TabletView.OutputColor); } catch (Exception) { }
                Color colorDraw = Colors.White;
                try { colorDraw = (Color)ColorConverter.ConvertFromString(config.TabletView.DrawColor); } catch (Exception) { }

                // Brushes
                brushInput = new SolidColorBrush(colorInput);
                brushInput.Freeze();
                brushOutput = new SolidColorBrush(colorOutput);
                brushOutput.Freeze();
                brushDraw = new SolidColorBrush(colorDraw);
                brushDraw.Freeze();

                // Line pens
                penInputLine = new Pen(brushInput, 3)
                {
                    StartLineCap = PenLineCap.Round,
                    EndLineCap = PenLineCap.Round,
                    LineJoin = PenLineJoin.Round
                };
                penInputLine.Freeze();
                penOutputLine = new Pen(brushOutput, 3)
                {
                    StartLineCap = PenLineCap.Round,
                    EndLineCap = PenLineCap.Round,
                    LineJoin = PenLineJoin.Round
                };
                penOutputLine.Freeze();
                penDrawLine = new Pen(brushDraw, 3)
                {
                    StartLineCap = PenLineCap.Round,
                    EndLineCap = PenLineCap.Round,
                    LineJoin = PenLineJoin.Round
                };
                penDrawLine.Freeze();

                // Circle pens
                penInputCircle = new Pen(brushInput, 3);
                penInputCircle.Freeze();
                penOutputCircle = new Pen(brushOutput, 3);
                penOutputCircle.Freeze();

                // Pressure border pens
                penInputPressure = new Pen(brushInput, 3)
                {
                    StartLineCap = PenLineCap.Round,
                    EndLineCap = PenLineCap.Round
                };
                penInputPressure.Freeze();
                penOutputPressure = new Pen(brushOutput, 3)
                {
                    StartLineCap = PenLineCap.Round,
                    EndLineCap = PenLineCap.Round
                };
                penOutputPressure.Freeze();

                // Rects
                rectInputPressure =
                    new Rect(120, 60, 200, 20);
                rectInputPressureBorder =
                    new Rect(120, 60, 200, 20);
                rectOutputPressure =
                    new Rect(120, 90, 200, 20);
                rectOutputPressureBorder =
                    new Rect(120, 90, 200, 20);

                // Offset pressure bars
                rectInputPressure.X += config.TabletView.OffsetPressure.X;
                rectInputPressure.Y += config.TabletView.OffsetPressure.Y;
                rectInputPressureBorder.X += config.TabletView.OffsetPressure.X;
                rectInputPressureBorder.Y += config.TabletView.OffsetPressure.Y;
                rectOutputPressure.X += config.TabletView.OffsetPressure.X;
                rectOutputPressure.Y += config.TabletView.OffsetPressure.Y;
                rectOutputPressureBorder.X += config.TabletView.OffsetPressure.X;
                rectOutputPressureBorder.Y += config.TabletView.OffsetPressure.Y;

            }


            //
            // Update last position arrays
            //
            public void UpdateLastPositions()
            {
                if (LastPositionsInput.Length != 0)
                {
                    Array.Copy(LastPositionsInput, 1, LastPositionsInput, 0, LastPositionsInput.Length - 1);
                    LastPositionsInput[LastPositionsInput.Length - 1].X = PositionInput.X;
                    LastPositionsInput[LastPositionsInput.Length - 1].Y = PositionInput.Y;
                }
                if (LastPositionsOutput.Length != 0)
                {
                    Array.Copy(LastPositionsOutput, 1, LastPositionsOutput, 0, LastPositionsOutput.Length - 1);
                    LastPositionsOutput[LastPositionsOutput.Length - 1].X = PositionOutput.X;
                    LastPositionsOutput[LastPositionsOutput.Length - 1].Y = PositionOutput.Y;
                }
            }


            //
            // Update draw position array
            //
            public void AddDrawPosition(double x, double y)
            {
                if (DrawPositions.Length == 0) return;

                // Shift position array
                Array.Copy(DrawPositions, 1, DrawPositions, 0, DrawPositions.Length - 1);

                // Set currect position as last item in the array
                DrawPositions[DrawPositions.Length - 1].X = x;
                DrawPositions[DrawPositions.Length - 1].Y = y;
            }


            //
            // Render 
            //
            protected override void OnRender(DrawingContext context)
            {
                PathFigure path;
                PathGeometry geometry;

                // Pressure bars
                rectInputPressure.Width = PressureInput * rectInputPressureBorder.Width;
                rectOutputPressure.Width = PressureOutput * rectInputPressureBorder.Width;
                context.DrawRectangle(brushInput, null, rectInputPressure);
                context.DrawRectangle(brushOutput, null, rectOutputPressure);
                context.DrawRectangle(null, penInputPressure, rectInputPressureBorder);
                context.DrawRectangle(null, penOutputPressure, rectOutputPressureBorder);

                //
                // Drawing line
                //
                if (DrawPositions.Length > 0)
                {
                    path = new PathFigure
                    {
                        StartPoint = DrawPositions[0]
                    };
                    for (int i = 1; i < DrawPositions.Length; i++)
                    {
                        // Last empty
                        if (DrawPositions[i].X >= 0 && DrawPositions[i - 1].X <= 0)
                        {
                            path.Segments.Add(new LineSegment(DrawPositions[i], false));
                            path.Segments.Add(new LineSegment(DrawPositions[i], true));
                        }
                        // Current and last OK
                        else if (DrawPositions[i].X >= 0 && DrawPositions[i - 1].X >= 0)
                            path.Segments.Add(new LineSegment(DrawPositions[i], true));

                        // Current and last empty
                        else
                            path.Segments.Add(new LineSegment(DrawPositions[i], false));
                    }
                    path.Freeze();
                    geometry = new PathGeometry();
                    geometry.Figures.Add(path);
                    geometry.Freeze();
                    context.DrawGeometry(null, penDrawLine, geometry);
                }

                //
                // Input line
                //
                if (LastPositionsInput.Length > 0)
                {
                    path = new PathFigure
                    {
                        StartPoint = LastPositionsInput[0]
                    };
                    for (int i = 1; i < LastPositionsInput.Length; i++)
                        path.Segments.Add(new LineSegment(LastPositionsInput[i], true));
                    path.Freeze();
                    geometry = new PathGeometry();
                    geometry.Figures.Add(path);
                    geometry.Freeze();
                    context.DrawGeometry(null, penInputLine, geometry);
                }


                //
                // Output line
                //
                if (LastPositionsOutput.Length > 0)
                {
                    path = new PathFigure
                    {
                        IsClosed = false,
                        IsFilled = false,
                        StartPoint = LastPositionsOutput[0]
                    };
                    for (int i = 1; i < LastPositionsOutput.Length; i++)
                        path.Segments.Add(new LineSegment(LastPositionsOutput[i], true));
                    path.Freeze();
                    geometry = new PathGeometry();
                    geometry.Figures.Add(path);
                    geometry.Freeze();
                    context.DrawGeometry(null, penOutputLine, geometry);
                }

                // Input circle
                context.DrawEllipse(null, penInputCircle, PositionInput, 12, 12);

                // Output circle
                context.DrawEllipse(null, penOutputCircle, PositionOutput, 12, 12);

            }

        }

        //
        // Tablet View Constructor
        //
        public WindowTabletView(Configuration config, TabletDriver driver)
        {
            if(config.TabletView.Borderless)
            {
                WindowStyle = WindowStyle.None;
            }
            InitializeComponent();

            this.config = config;
            this.driver = driver;

            // Tablet renderer
            tabletRenderer = new TabletRenderer(config);
            canvasTabletView.Children.Add(tabletRenderer);

            // Offset texts
            Canvas.SetLeft(textTabletInfo, Canvas.GetLeft(textTabletInfo) + config.TabletView.OffsetText.X);
            Canvas.SetTop(textTabletInfo, Canvas.GetTop(textTabletInfo) + config.TabletView.OffsetText.Y);
            Canvas.SetLeft(textInput, Canvas.GetLeft(textInput) + config.TabletView.OffsetText.X);
            Canvas.SetTop(textInput, Canvas.GetTop(textInput) + config.TabletView.OffsetText.Y);
            Canvas.SetLeft(textOutput, Canvas.GetLeft(textOutput) + config.TabletView.OffsetText.X);
            Canvas.SetTop(textOutput, Canvas.GetTop(textOutput) + config.TabletView.OffsetText.Y);
            Canvas.SetLeft(textLatency, Canvas.GetLeft(textLatency) + config.TabletView.OffsetText.X);
            Canvas.SetTop(textLatency, Canvas.GetTop(textLatency) + config.TabletView.OffsetText.Y);

            // Background color
            Brush brush;
            try { brush = new SolidColorBrush((Color)ColorConverter.ConvertFromString(config.TabletView.BackgroundColor)); }
            catch (Exception) { brush = Brushes.White; }
            canvasTabletView.Background = brush;
            Background = brush;

            // Text colors
            try { brush = new SolidColorBrush((Color)ColorConverter.ConvertFromString(config.TabletView.InfoColor)); }
            catch (Exception) { brush = Brushes.Black; }
            textTabletInfo.Foreground = brush;
            try { brush = new SolidColorBrush((Color)ColorConverter.ConvertFromString(config.TabletView.LatencyColor)); }
            catch (Exception) { brush = Brushes.Black; }
            textLatency.Foreground = brush;
            textInput.Foreground = tabletRenderer.brushInput;
            textOutput.Foreground = tabletRenderer.brushOutput;

            // Text font
            try
            {
                FontFamilyConverter fontConverter = new FontFamilyConverter();
                FontFamily fontFamily = (FontFamily)fontConverter.ConvertFromString(config.TabletView.Font);
                textTabletInfo.FontFamily = fontFamily;
                textInput.FontFamily = fontFamily;
                textOutput.FontFamily = fontFamily;
                textLatency.FontFamily = fontFamily;
            }
            catch (Exception) { }

            // Font size
            textTabletInfo.FontSize = config.TabletView.FontSize;
            textInput.FontSize = config.TabletView.FontSize;
            textOutput.FontSize = config.TabletView.FontSize;
            textLatency.FontSize = config.TabletView.FontSize;

            // Info text
            textTabletInfo.Text = config.TabletName + " - " +
                Utils.GetNumberString(config.TabletAreas[0].Width) + " x " +
                Utils.GetNumberString(config.TabletAreas[0].Height) + " mm → " +
                Utils.GetNumberString(config.ScreenAreas[0].Width, "0") + " x " +
                Utils.GetNumberString(config.ScreenAreas[0].Height, "0") + " px";


            //
            // Update/draw timer
            //
            timer = new MultimediaTimer { Interval = 2 };
            timer.Tick += UpdateTimer_Tick;

            // Last values
            lastPosition = new Vector(0, 0);
            lastUpdate = DateTime.Now;
            lastPressure = 0;

            // Average values
            velocity = 0;
            latency = 0;


            // Input loss
            hadInputLoss = true;
            lastInputStartTime = DateTime.Now;

            // Window events
            Loaded += WindowTabletView_Loaded;
            Closing += WindowTabletView_Closing;
            KeyDown += WindowTabletView_KeyDown;

            MouseDown += WindowTabletView_MouseDown;
            MouseUp += WindowTabletView_MouseUp;


            // Set GC mode to low latency
            GCSettings.LatencyMode = GCLatencyMode.SustainedLowLatency;

        }

        //
        // Update/draw timer tick
        //
        private void UpdateTimer_Tick(object sender, EventArgs e)
        {
            // Output position haven't changed -> skip
            if (
                lastPosition.X == driver.tabletState.outputX
                &&
                lastPosition.Y == driver.tabletState.outputY
            )
            {
                double timeDelta = (DateTime.Now - lastUpdate).TotalMilliseconds;

                // Input loss
                if (timeDelta >= 30)
                {
                    hadInputLoss = true;
                }

                // Hide renderer
                if (
                    config.TabletView.FadeInOut
                    &&
                    timeDelta >= 200
                    &&
                    timeDelta <= 1000
                )
                {
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        canvasTabletView.Opacity -= (timeDelta - 200) / 5000.0;
                        if (canvasTabletView.Opacity < 0)
                            canvasTabletView.Opacity = 0;

                        //canvasTabletView.Opacity = 1.0 - ((timeDelta - 200.0) / 800.0);
                        canvasTabletView.InvalidateVisual();
                    });
                }


                return;
            }
            else
            {
                if (hadInputLoss)
                {
                    lastInputStartTime = DateTime.Now;
                    hadInputLoss = false;
                }
            }

            //
            // Update renderer positions
            //
            // Inverted
            if (config.Invert)
            {
                tabletRenderer.PositionInput.X = config.TabletFullArea.Width - driver.tabletState.inputX - config.TabletAreas[0].X;
                tabletRenderer.PositionInput.Y = config.TabletFullArea.Height - driver.tabletState.inputY - config.TabletAreas[0].Y;
                tabletRenderer.PositionOutput.X = config.TabletFullArea.Width - driver.tabletState.outputX - config.TabletAreas[0].X;
                tabletRenderer.PositionOutput.Y = config.TabletFullArea.Height - driver.tabletState.outputY - config.TabletAreas[0].Y;
            }
            // Normal
            else
            {
                tabletRenderer.PositionInput.X = driver.tabletState.inputX - config.TabletAreas[0].X;
                tabletRenderer.PositionInput.Y = driver.tabletState.inputY - config.TabletAreas[0].Y;
                tabletRenderer.PositionOutput.X = driver.tabletState.outputX - config.TabletAreas[0].X;
                tabletRenderer.PositionOutput.Y = driver.tabletState.outputY - config.TabletAreas[0].Y;
            }

            // Rotate and offset positions
            Point rotatedPoint = new Point(0, 0);
            config.TabletAreas[0].GetRotatedPointReverse(ref rotatedPoint, tabletRenderer.PositionInput.X, tabletRenderer.PositionInput.Y);
            tabletRenderer.PositionInput.X = rotatedPoint.X + config.TabletAreas[0].Width / 2.0;
            tabletRenderer.PositionInput.Y = rotatedPoint.Y + config.TabletAreas[0].Height / 2.0;

            config.TabletAreas[0].GetRotatedPointReverse(ref rotatedPoint, tabletRenderer.PositionOutput.X, tabletRenderer.PositionOutput.Y);
            tabletRenderer.PositionOutput.X = rotatedPoint.X + config.TabletAreas[0].Width / 2.0;
            tabletRenderer.PositionOutput.Y = rotatedPoint.Y + config.TabletAreas[0].Height / 2.0;


            // Scale to canvas
            tabletRenderer.PositionInput.X *= tabletRenderer.ScaleX;
            tabletRenderer.PositionInput.Y *= tabletRenderer.ScaleY;
            tabletRenderer.PositionOutput.X *= tabletRenderer.ScaleX;
            tabletRenderer.PositionOutput.Y *= tabletRenderer.ScaleY;


            // Update renderer pressures
            tabletRenderer.PressureInput = driver.tabletState.inputPressure;
            tabletRenderer.PressureOutput = driver.tabletState.outputPressure;

            //
            // Run in the main thread
            //
            Application.Current.Dispatcher.Invoke(() =>
            {
                // Show renderer
                if (config.TabletView.FadeInOut)
                {
                    double timeDelta = (DateTime.Now - lastInputStartTime).TotalMilliseconds;
                    if (timeDelta >= 0 && timeDelta <= 800 || canvasTabletView.Opacity >= 1)
                    {
                        canvasTabletView.Opacity += timeDelta / 5000.0;
                        if (canvasTabletView.Opacity >= 1)
                            canvasTabletView.Opacity = 1;
                    }
                    else
                    {
                        canvasTabletView.Opacity = 1.0;
                    }
                }

                // Update pen positions
                tabletRenderer.UpdateLastPositions();

                // Add draw positions
                if (driver.tabletState.outputPressure > 0)
                {
                    if (lastPressure <= 0)
                    {
                        tabletRenderer.AddDrawPosition(-1, -1);
                    }
                    tabletRenderer.AddDrawPosition(tabletRenderer.PositionOutput.X, tabletRenderer.PositionOutput.Y);
                }
                lastPressure = driver.tabletState.outputPressure;

                // Latency text
                if (driver.tabletState.inputVelocity > 0)
                {
                    double dx = driver.tabletState.inputX - driver.tabletState.outputX;
                    double dy = driver.tabletState.inputY - driver.tabletState.outputY;
                    double distance = Math.Sqrt(dx * dx + dy * dy);
                    double targetLatency = distance / driver.tabletState.inputVelocity * 1000.0;
                    latency += (targetLatency - latency) / 10.0;
                    velocity += (driver.tabletState.inputVelocity - velocity) / 20.0;
                    textLatency.Text = "±" + Utils.GetNumberString(latency, "0") + " ms\n" +
                    Utils.GetNumberString(Math.Round(velocity / 10.0) * 10, "0") + " mm/s";
                }

                // Render
                tabletRenderer.InvalidateVisual();

                // Update last values
                lastPosition.X = driver.tabletState.outputX;
                lastPosition.Y = driver.tabletState.outputY;
                lastUpdate = DateTime.Now;
            });

        }


        //
        // Window key down
        //
        private void WindowTabletView_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
            {
                Close();
            }
            if (e.Key == Key.B)
            {
                if (canvasTabletView.Background == Brushes.White)
                    canvasTabletView.Background = Brushes.Black;
                else
                    canvasTabletView.Background = Brushes.White;
            }
        }


        //
        // Window loaded
        //
        private void WindowTabletView_Loaded(object sender, RoutedEventArgs e)
        {
            double scaleX, scaleY;
            double aspectScreen, aspectTablet;
            double newHeight;

            // Canvas default size
            canvasTabletView.Width = 1280;
            canvasTabletView.Height = 720;

            // Tablet renderer
            scaleX = canvasTabletView.Width / config.TabletAreas[0].Width;

            // Set canvas height
            newHeight = config.TabletAreas[0].Height * scaleX;
            aspectScreen = config.ScreenAreas[0].Width / config.ScreenAreas[0].Height;
            aspectTablet = config.TabletAreas[0].Width / config.TabletAreas[0].Height;
            newHeight *= aspectTablet / aspectScreen;
            scaleY = newHeight / config.TabletAreas[0].Height;
            canvasTabletView.Height = newHeight;

            // Set renderer scale
            tabletRenderer.ScaleX = scaleX;
            tabletRenderer.ScaleY = scaleY;

            // Timer 
            timer.Start();

            // Enable state output
            driver.SendCommand("StateOutput true");
        }

        //
        // Window closed
        //
        private void WindowTabletView_Closing(object sender, EventArgs e)
        {
            if (timer.IsRunning)
                timer.Stop();

            // Disable state output
            driver.SendCommand("StateOutput false");

            // Set GC mode back to normal
            GCSettings.LatencyMode = GCLatencyMode.Interactive;
            GC.Collect(10, GCCollectionMode.Forced);
        }

        //
        // Window mouse down
        //
        private void WindowTabletView_MouseDown(object sender, MouseButtonEventArgs e)
        {

            // Drag move with left mouse button in borderless mode
            if (config.TabletView.Borderless && e.LeftButton == MouseButtonState.Pressed)
            {
                DragMove();
            }
        }

        //
        // Window mouse up
        //
        private void WindowTabletView_MouseUp(object sender, MouseButtonEventArgs e)
        {
            // Close window with right click in borderless mode
            if (config.TabletView.Borderless && e.ChangedButton == MouseButton.Right && e.RightButton == MouseButtonState.Released)
            {
                Close();
            }
        }
    }
}
