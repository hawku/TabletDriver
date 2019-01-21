using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace TabletDriverGUI
{
    public partial class MainWindow : Window
    {

        //
        // Screen map canvas elements
        //
        private Rectangle[] rectangleMonitors;
        private Rectangle rectangleDesktop;
        private Rectangle[] rectangleScreenAreas;
        private Image imageDesktopScreenshot;
        private Area lastDesktopSize;
        private Matrix matrixScreenAreaToCanvas;
        private Matrix matrixCanvasToScreenArea;

        //
        // Tablet area canvas elements
        //
        private Polygon polygonTabletFullArea;
        private Polygon[] polygonTabletAreas;
        private Polygon polygonTabletAreaArrow;
        private Ellipse ellipsePenPosition;
        private Ellipse ellipsePenPosition2;
        private int lastPenPositionIndex;
        private Matrix matrixTabletAreaToCanvas;
        private Matrix matrixCanvasToTabletArea;

        // Area colors
        private Brush[] brushBackgrounds;
        private Brush brushStrokeSelected;
        private Brush brushStrokeNormal;
        private static double selectedStrokeThickness = 4.0;


        // Canvas clicked area
        private class MouseArea
        {
            public Area Area;
            public Point Point;
            public int Index;
            public bool IsValid;
            public MouseArea()
            {
                Area = null;
                Point = new Point(0, 0);
                Index = 0;
                IsValid = false;
            }
        }
        private MouseArea mouseArea;


        // Canvas Mouse drag
        private class MouseDrag
        {
            public bool IsMouseDown;
            public object Source;
            public Point OriginMouse;
            public Point OriginDraggable;
            public Area DragArea;
            public MouseDrag()
            {
                IsMouseDown = false;
                Source = null;
                OriginMouse = new Point(0, 0);
                OriginDraggable = new Point(0, 0);
            }
        }
        MouseDrag mouseDrag;



        //
        // Get desktop size
        //
        System.Drawing.Rectangle GetVirtualDesktopSize()
        {
            System.Drawing.Rectangle rect = new System.Drawing.Rectangle();

            // Windows 8 or greater needed for the multiscreen absolute mode
            if (VersionHelper.IsWindows8OrGreater())
            {
                rect.Width = System.Windows.Forms.SystemInformation.VirtualScreen.Width;
                rect.Height = System.Windows.Forms.SystemInformation.VirtualScreen.Height;
            }
            else if (config.Mode != Configuration.OutputModes.Compatibility)
            {
                rect.Width = System.Windows.Forms.SystemInformation.VirtualScreen.Width;
                rect.Height = System.Windows.Forms.SystemInformation.VirtualScreen.Height;
            }
            else
            {
                rect.Width = System.Windows.Forms.Screen.PrimaryScreen.Bounds.Width;
                rect.Height = System.Windows.Forms.Screen.PrimaryScreen.Bounds.Height;

            }
            return rect;
        }


        //
        // Get available screens
        //
        System.Windows.Forms.Screen[] GetAvailableScreens()
        {
            System.Windows.Forms.Screen[] screens;

            // Windows 8 or greater needed for the multiscreen absolute mode
            if (VersionHelper.IsWindows8OrGreater())
                screens = System.Windows.Forms.Screen.AllScreens;
            else if (config.Mode != Configuration.OutputModes.Compatibility)
                screens = System.Windows.Forms.Screen.AllScreens;
            else
                screens = new System.Windows.Forms.Screen[] { System.Windows.Forms.Screen.PrimaryScreen };
            return screens;
        }

        //
        // Get minimum screen position
        //
        Vector GetMinimumScreenPosition(System.Windows.Forms.Screen[] screens)
        {

            // Monitor minimums
            double minX = 0;
            double minY = 0;
            bool first = true;
            foreach (System.Windows.Forms.Screen screen in screens)
            {
                if (first)
                {
                    minX = screen.Bounds.X;
                    minY = screen.Bounds.Y;
                    first = false;
                }
                else
                {
                    if (screen.Bounds.X < minX) minX = screen.Bounds.X;
                    if (screen.Bounds.Y < minY) minY = screen.Bounds.Y;
                }
            }

            return new Vector(minX, minY);
        }

        //
        // Fix tablet area dimension
        //
        void FixTabletAreaDimensions(Area tabletArea, Area screenArea)
        {
            // Limits
            if (tabletArea.Width > config.TabletFullArea.Width)
                tabletArea.Width = config.TabletFullArea.Width;
            if (tabletArea.Height > config.TabletFullArea.Height)
                tabletArea.Height = config.TabletFullArea.Height;

            // Aspect ratio
            if (config.ForceAspectRatio)
            {
                double aspectRatio = screenArea.Width / screenArea.Height;
                tabletArea.Height = tabletArea.Width / aspectRatio;
                if (tabletArea.Height > config.TabletFullArea.Height)
                {
                    tabletArea.Height = config.TabletFullArea.Height;
                    tabletArea.Width = tabletArea.Height * aspectRatio;
                }
            }
        }


        //
        // Create canvas elements
        //
        void CreateCanvasElements()
        {
            //
            // Screen map canvas
            //
            // Clear canvas
            canvasScreenMap.Children.Clear();

            //
            // Area background brushes
            //
            brushBackgrounds = new SolidColorBrush[10];
            Color[] colors = new Color[10];
            for (int i = 0; i < colors.Length; i++)
            {
                colors[i] = Color.FromArgb(20, 20, 20, 20);
            }

            colors[0] = Color.FromRgb(186, 255, 201);
            colors[1] = Color.FromRgb(186, 225, 255);
            colors[2] = Color.FromRgb(255, 223, 186);
            colors[3] = Color.FromRgb(255, 179, 186);
            colors[4] = Color.FromRgb(255, 255, 150);

            for (int i = 0; i < brushBackgrounds.Length; i++)
            {
                double multiplier = 0.9;
                colors[i].R = (byte)(Math.Round(colors[i].R * multiplier));
                colors[i].G = (byte)(Math.Round(colors[i].G * multiplier));
                colors[i].B = (byte)(Math.Round(colors[i].B * multiplier));
                colors[i].A = 128;
                brushBackgrounds[i] = new SolidColorBrush(colors[i]);
            }

            //
            // Selected brushes
            //
            brushStrokeSelected = new SolidColorBrush(Color.FromArgb(128, 255, 30, 30));
            brushStrokeNormal = new SolidColorBrush(Color.FromArgb(255, 0, 0, 0));


            //
            // Screen map desktop screenshot
            //
            imageDesktopScreenshot = new Image
            {
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center,
                Width = 100,
                Height = 100,
                Opacity = 0.7
            };
            canvasScreenMap.Children.Add(imageDesktopScreenshot);
            lastDesktopSize = new Area(0, 0, 0, 0);


            // Monitor rectangles
            rectangleMonitors = new Rectangle[16];
            for (int i = 0; i < 16; i++)
            {
                rectangleMonitors[i] = new Rectangle
                {
                    Width = 10,
                    Height = 10,
                    Stroke = Brushes.Black,
                    StrokeThickness = 0.5,
                    Fill = Brushes.Transparent,
                    Visibility = Visibility.Collapsed
                };
                canvasScreenMap.Children.Add(rectangleMonitors[i]);
            }

            //
            // Desktop area rectangle
            //
            rectangleDesktop = new Rectangle
            {
                Stroke = Brushes.Black,
                StrokeThickness = 0.5,
                Fill = Brushes.Transparent
            };
            canvasScreenMap.Children.Add(rectangleDesktop);


            //
            // Screen map area rectangles
            //
            rectangleScreenAreas = new Rectangle[config.GetMaxAreaCount()];
            for (int i = config.GetMaxAreaCount() - 1; i >= 0; i--)
            {
                rectangleScreenAreas[i] = new Rectangle
                {
                    Stroke = Brushes.Black,
                    StrokeThickness = 1.5,
                    Fill = brushBackgrounds[i],
                    Cursor = Cursors.Hand,
                    Visibility = Visibility.Collapsed
                };
                canvasScreenMap.Children.Add(rectangleScreenAreas[i]);
            }

            // Screen area matrix
            matrixScreenAreaToCanvas = new Matrix(0, 0, 0, 0, 0, 0);
            matrixCanvasToScreenArea = new Matrix(0, 0, 0, 0, 0, 0);

            //
            // Tablet area canvas
            //
            //
            // Clear
            canvasTabletArea.Children.Clear();

            //
            // Tablet full area polygon
            //
            polygonTabletFullArea = new Polygon
            {
                Stroke = new SolidColorBrush(Color.FromRgb(100, 100, 100)),
                StrokeThickness = 2.0,
                Points = new PointCollection
                {
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0)
                }
            };
            canvasTabletArea.Children.Add(polygonTabletFullArea);

            //
            // Tablet area polygons
            //
            polygonTabletAreas = new Polygon[config.GetMaxAreaCount()];
            for (int i = config.GetMaxAreaCount() - 1; i >= 0; i--)
            {
                polygonTabletAreas[i] = new Polygon
                {
                    Stroke = Brushes.Black,
                    StrokeLineJoin = PenLineJoin.Round,
                    Fill = brushBackgrounds[i],
                    StrokeThickness = 1.5,
                    Cursor = Cursors.Hand,
                    Points = new PointCollection
                {
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0)
                },
                };
                canvasTabletArea.Children.Add(polygonTabletAreas[i]);
            }


            //
            // Tablet area arrow polygon
            //
            polygonTabletAreaArrow = new Polygon
            {
                Fill = new SolidColorBrush(Color.FromArgb(50, 20, 20, 20)),
                Cursor = Cursors.Hand,
                Points = new PointCollection
                {
                    new Point(0,0),
                    new Point(0,0),
                    new Point(0,0)
                },
            };
            canvasTabletArea.Children.Add(polygonTabletAreaArrow);

            //
            // Tablet area pen position
            //
            ellipsePenPosition = new Ellipse
            {
                Stroke = Brushes.Green,
                StrokeThickness = 1,
                Width = 5,
                Height = 5,
                Visibility = Visibility.Collapsed
            };
            canvasTabletArea.Children.Add(ellipsePenPosition);

            ellipsePenPosition2 = new Ellipse
            {
                Stroke = Brushes.Red,
                StrokeThickness = 1,
                Width = 5,
                Height = 5,
                Visibility = Visibility.Collapsed
            };
            canvasTabletArea.Children.Add(ellipsePenPosition2);


            timerUpdatePenPositions = new DispatcherTimer
            {
                Interval = new TimeSpan(0, 0, 0, 0, 20)
            };
            lastPenPositionIndex = 0;
            timerUpdatePenPositions.Tick += (sender, e) =>
            {
                if (lastPenPositionIndex == 0)
                {
                    ellipsePenPosition.Visibility = Visibility.Visible;
                    ellipsePenPosition2.Visibility = Visibility.Visible;
                }
                if (driver.tabletState.index != lastPenPositionIndex)
                {
                    UpdateTabletAreaCanvasPenPositions();
                    lastPenPositionIndex = driver.tabletState.index;
                }
            };
            timerUpdatePenPositions.Stop();


            //
            // Tablet area matrix
            //
            matrixTabletAreaToCanvas = new Matrix(0, 0, 0, 0, 0, 0);
            matrixCanvasToTabletArea = new Matrix(0, 0, 0, 0, 0, 0);

            // Canvas mouse area
            mouseArea = new MouseArea();

            //
            // Canvas mouse drag
            //
            mouseDrag = new MouseDrag();
        }


        //
        // Update canvas elements
        //
        void UpdateCanvasElements()
        {
            if (config == null) return;
            if (isLoadingSettings) return;
            UpdateDesktopImage();
            UpdateScreenMapCanvas();
            UpdateTabletAreaCanvas();
            UpdateAreaInformation();
        }


        //
        // Update screen map desktop image
        //
        void UpdateDesktopImage()
        {
            if (imageDesktopScreenshot != null && imageDesktopScreenshot.Source != null)
            {
                if (
                    config.DesktopSize.Width == lastDesktopSize.Width
                    &&
                    config.DesktopSize.Height == lastDesktopSize.Height
                )
                {
                    return;
                }
            }

            try
            {
                int screenLeft = System.Windows.Forms.SystemInformation.VirtualScreen.Left;
                int screenTop = System.Windows.Forms.SystemInformation.VirtualScreen.Top;
                int screenWidth = System.Windows.Forms.SystemInformation.VirtualScreen.Width;
                int screenHeight = System.Windows.Forms.SystemInformation.VirtualScreen.Height;


                // Create desktop screenshot bitmap
                System.Drawing.Bitmap bitmapDesktop = new System.Drawing.Bitmap(screenWidth, screenHeight);
                System.Drawing.Graphics graphicsDesktop = System.Drawing.Graphics.FromImage(bitmapDesktop);
                graphicsDesktop.CopyFromScreen(screenLeft, screenTop, 0, 0, bitmapDesktop.Size, System.Drawing.CopyPixelOperation.SourceCopy);


                // Create downscaled bitmap
                double scaleX = canvasScreenMap.ActualWidth / screenWidth;
                double scaleY = canvasScreenMap.ActualHeight / screenHeight;
                double scale = scaleX;
                if (scaleX > scaleY)
                    scale = scaleY;
                System.Drawing.Bitmap bitmapDownscaled = new System.Drawing.Bitmap(
                    (int)Math.Round(screenWidth * scale),
                    (int)Math.Round(screenHeight * scale)
                );
                System.Drawing.Graphics graphicsDownscaled = System.Drawing.Graphics.FromImage(bitmapDownscaled);
                graphicsDownscaled.DrawImage(bitmapDesktop, 0, 0, bitmapDownscaled.Width, bitmapDownscaled.Height);


                // Create source from the bitmap
                IntPtr handleBitmap = bitmapDownscaled.GetHbitmap();
                BitmapSource bitmapSource = System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap(
                        handleBitmap,
                        IntPtr.Zero,
                        Int32Rect.Empty,
                        BitmapSizeOptions.FromEmptyOptions()
                    );
                imageDesktopScreenshot.Source = bitmapSource;

                // Release resources
                NativeMethods.DeleteObject(handleBitmap);
                graphicsDesktop.Dispose();
                bitmapDesktop.Dispose();
                graphicsDownscaled.Dispose();
                bitmapDownscaled.Dispose();

                // Run GC
                GC.Collect();
                GC.WaitForPendingFinalizers();

                // Update last desktop size
                lastDesktopSize.Width = config.DesktopSize.Width;
                lastDesktopSize.Height = config.DesktopSize.Height;

            }
            catch (Exception)
            {
            }
        }


        //
        // Update screen map canvas elements
        //
        void UpdateScreenMapCanvas()
        {

            // Canvas element scaling
            double scaleX = (canvasScreenMap.ActualWidth - 2) / config.DesktopSize.Width;
            double scaleY = (canvasScreenMap.ActualHeight - 2) / config.DesktopSize.Height;
            double scale = scaleX;
            if (scaleX > scaleY)
                scale = scaleY;

            // Centered offset
            double offsetX = canvasScreenMap.ActualWidth / 2.0 - config.DesktopSize.Width * scale / 2.0;
            double offsetY = canvasScreenMap.ActualHeight / 2.0 - config.DesktopSize.Height * scale / 2.0;

            // Matrices
            matrixScreenAreaToCanvas.M11 = scale;
            matrixScreenAreaToCanvas.M22 = scale;
            matrixScreenAreaToCanvas.OffsetX = offsetX;
            matrixScreenAreaToCanvas.OffsetY = offsetY;

            matrixCanvasToScreenArea.M11 = 1 / scale;
            matrixCanvasToScreenArea.M22 = 1 / scale;
            matrixCanvasToScreenArea.OffsetX = -offsetX / scale;
            matrixCanvasToScreenArea.OffsetY = -offsetY / scale;


            // Full desktop area
            rectangleDesktop.Width = config.DesktopSize.Width * scale;
            rectangleDesktop.Height = config.DesktopSize.Height * scale;
            Canvas.SetLeft(rectangleDesktop, offsetX);
            Canvas.SetTop(rectangleDesktop, offsetY);

            imageDesktopScreenshot.Width = config.DesktopSize.Width * scale;
            imageDesktopScreenshot.Height = config.DesktopSize.Height * scale;
            Canvas.SetLeft(imageDesktopScreenshot, offsetX);
            Canvas.SetTop(imageDesktopScreenshot, offsetY);


            //
            // Screen map area rectangles
            //
            for (int i = 0; i < config.GetAreaCount(); i++)
            {
                rectangleScreenAreas[i].Width = config.ScreenAreas[i].Width * scale;
                rectangleScreenAreas[i].Height = config.ScreenAreas[i].Height * scale;
                Canvas.SetLeft(rectangleScreenAreas[i], offsetX + (config.ScreenAreas[i].X - config.ScreenAreas[i].Width / 2.0) * scale);
                Canvas.SetTop(rectangleScreenAreas[i], offsetY + (config.ScreenAreas[i].Y - config.ScreenAreas[i].Height / 2.0) * scale);

                if (mouseArea.IsValid && mouseArea.Index == i)
                {
                    rectangleScreenAreas[i].Stroke = brushStrokeSelected;
                    rectangleScreenAreas[i].StrokeThickness = selectedStrokeThickness;
                }
                else
                {
                    rectangleScreenAreas[i].Stroke = brushStrokeNormal;
                    rectangleScreenAreas[i].StrokeThickness = 1.5;
                }

                if (config.ScreenAreas[i].IsEnabled)
                {
                    rectangleScreenAreas[i].Visibility = Visibility.Visible;
                }
                else
                {
                    rectangleScreenAreas[i].Visibility = Visibility.Collapsed;
                }
            }

            // Only primary area enabled -> remove red border
            if (config.GetEnabledAreaCount() == 1)
            {
                rectangleScreenAreas[0].Stroke = brushStrokeNormal;
                rectangleScreenAreas[0].StrokeThickness = 1.5;

            }

            //
            // Monitor rectangles
            //
            System.Windows.Forms.Screen[] screens = GetAvailableScreens();
            Vector minimumScreenPosition = GetMinimumScreenPosition(screens);

            // Hide all rectangles
            for (int i = 0; i < rectangleMonitors.Length; i++)
                rectangleMonitors[i].Visibility = Visibility.Collapsed;

            int rectangeIndex = 0;
            foreach (System.Windows.Forms.Screen screen in screens)
            {
                double x = screen.Bounds.X - minimumScreenPosition.X;
                double y = screen.Bounds.Y - minimumScreenPosition.Y;

                rectangleMonitors[rectangeIndex].Visibility = Visibility.Visible;
                rectangleMonitors[rectangeIndex].Width = screen.Bounds.Width * scale;
                rectangleMonitors[rectangeIndex].Height = screen.Bounds.Height * scale;
                Canvas.SetLeft(rectangleMonitors[rectangeIndex], offsetX + x * scale);
                Canvas.SetTop(rectangleMonitors[rectangeIndex], offsetY + y * scale);

                rectangeIndex++;
                if (rectangeIndex >= 16) break;
            }


            canvasScreenMap.InvalidateVisual();

        }


        //
        // Update tablet area canvas elements
        //
        void UpdateTabletAreaCanvas()
        {
            double fullWidth = config.TabletFullArea.Width;
            double fullHeight = config.TabletFullArea.Height;

            // Canvas element scaling
            double scaleX = (canvasTabletArea.ActualWidth - 2) / fullWidth;
            double scaleY = (canvasTabletArea.ActualHeight - 2) / fullHeight;
            double scale = scaleX;
            if (scaleX > scaleY)
                scale = scaleY;

            double offsetX = canvasTabletArea.ActualWidth / 2.0 - fullWidth * scale / 2.0;
            double offsetY = canvasTabletArea.ActualHeight / 2.0 - fullHeight * scale / 2.0;

            // Matrices
            matrixTabletAreaToCanvas.M11 = scale;
            matrixTabletAreaToCanvas.M22 = scale;
            matrixTabletAreaToCanvas.OffsetX = offsetX;
            matrixTabletAreaToCanvas.OffsetY = offsetY;

            matrixCanvasToTabletArea.M11 = 1 / scale;
            matrixCanvasToTabletArea.M22 = 1 / scale;
            matrixCanvasToTabletArea.OffsetX = -offsetX / scale;
            matrixCanvasToTabletArea.OffsetY = -offsetY / scale;



            //
            // Tablet full area
            //
            Point[] corners = config.TabletFullArea.Corners;
            for (int i = 0; i < 4; i++)
            {
                Point p = corners[i];
                p.X *= scale;
                p.Y *= scale;
                p.X += config.TabletFullArea.X * scale + offsetX;
                p.Y += config.TabletFullArea.Y * scale + offsetY;
                polygonTabletFullArea.Points[i] = p;
            }


            //
            // Tablet areas
            //
            for (int i = 0; i < config.GetAreaCount(); i++)
            {
                corners = config.TabletAreas[i].Corners;
                for (int j = 0; j < 4; j++)
                {
                    Point p = corners[j];
                    p.X *= scale;
                    p.Y *= scale;
                    p.X += config.TabletAreas[i].X * scale + offsetX;
                    p.Y += config.TabletAreas[i].Y * scale + offsetY;
                    polygonTabletAreas[i].Points[j] = p;
                }


                if (mouseArea.IsValid && mouseArea.Index == i)
                {
                    polygonTabletAreas[i].Stroke = brushStrokeSelected;
                    polygonTabletAreas[i].StrokeThickness = selectedStrokeThickness;
                }
                else
                {
                    polygonTabletAreas[i].Stroke = brushStrokeNormal;
                    polygonTabletAreas[i].StrokeThickness = 1.5;
                }

                if (config.ScreenAreas[i].IsEnabled)
                {
                    polygonTabletAreas[i].Visibility = Visibility.Visible;
                }
                else
                {
                    polygonTabletAreas[i].Visibility = Visibility.Collapsed;
                }
            }

            // Only primary area enabled -> remove red border
            if (config.GetEnabledAreaCount() == 1)
            {
                polygonTabletAreas[0].Stroke = brushStrokeNormal;
                polygonTabletAreas[0].StrokeThickness = 1.5;
            }



            //
            // Tablet area arrow
            //
            corners = config.TabletAreas[0].Corners;
            polygonTabletAreaArrow.Points[0] = new Point(
                offsetX + config.TabletAreas[0].X * scale,
                offsetY + config.TabletAreas[0].Y * scale
            );

            polygonTabletAreaArrow.Points[1] = new Point(
                offsetX + corners[2].X * scale + config.TabletAreas[0].X * scale,
                offsetY + corners[2].Y * scale + config.TabletAreas[0].Y * scale
            );

            polygonTabletAreaArrow.Points[2] = new Point(
                offsetX + corners[3].X * scale + config.TabletAreas[0].X * scale,
                offsetY + corners[3].Y * scale + config.TabletAreas[0].Y * scale
            );


            canvasTabletArea.InvalidateVisual();

        }



        //
        // Update canvas pen positions
        // 
        void UpdateTabletAreaCanvasPenPositions()
        {
            Point p = new Point();

            // Inverted
            if (config.Invert)
            {
                // Input position
                p.X = config.TabletFullArea.Width - driver.tabletState.inputX;
                p.Y = config.TabletFullArea.Height - driver.tabletState.inputY;
                matrixTabletAreaToCanvas.Transform(p);
                Canvas.SetLeft(ellipsePenPosition, p.X);
                Canvas.SetTop(ellipsePenPosition, p.Y);

                // Output position
                p.X = config.TabletFullArea.Width - driver.tabletState.outputX;
                p.Y = config.TabletFullArea.Height - driver.tabletState.outputY;
                matrixTabletAreaToCanvas.Transform(p);
                Canvas.SetLeft(ellipsePenPosition2, p.X);
                Canvas.SetTop(ellipsePenPosition2, p.Y);
            }
            else
            {
                // Input position
                p.X = driver.tabletState.inputX;
                p.Y = driver.tabletState.inputY;
                matrixTabletAreaToCanvas.Transform(p);
                Canvas.SetLeft(ellipsePenPosition, p.X);
                Canvas.SetTop(ellipsePenPosition, p.Y);

                // Output position
                p.X = driver.tabletState.outputX;
                p.Y = driver.tabletState.outputY;
                matrixTabletAreaToCanvas.Transform(p);
                Canvas.SetLeft(ellipsePenPosition2, p.X);
                Canvas.SetTop(ellipsePenPosition2, p.Y);
            }

        }

        //
        // Get area coordinates from canvas position
        //
        Point GetAreaCoordinates(UIElement canvas, Point p)
        {
            Point point = new Point(p.X, p.Y);

            if (canvas == canvasScreenMap)
            {
                point = matrixCanvasToScreenArea.Transform(point);
            }
            else if (canvas == canvasTabletArea)
            {
                point = matrixCanvasToTabletArea.Transform(point);
            }

            return point;
        }


        //
        // Update last clicked area
        //
        void UpdateMouseArea(UIElement sender, Point cursorPosition)
        {
            Area area = null;
            Point areaPoint = new Point(0, 0);
            int index = -1;

            Point clickPosition = GetAreaCoordinates(sender, cursorPosition);

            //
            // Screen areas
            //
            if (sender == canvasScreenMap)
            {
                for (int i = 0; i < config.GetAreaCount(); i++)
                {
                    if (
                        config.ScreenAreas[i].IsEnabled
                        &&
                        config.ScreenAreas[i].IsInside(clickPosition)
                        &&

                        // Area smaller than last found area?
                        (
                            area == null
                            ||
                            area.Width > config.ScreenAreas[i].Width
                        )
                    )
                    {
                        area = config.ScreenAreas[i];
                        config.SelectedScreenArea = config.ScreenAreas[i];
                        config.SelectedTabletArea = config.TabletAreas[i];
                        index = i;
                    }
                }
                if (index == -1)
                {
                    config.SelectedScreenArea = config.ScreenAreas[0];
                    config.SelectedTabletArea = config.TabletAreas[0];
                }
            }


            //
            // Tablet areas
            //
            else if (sender == canvasTabletArea)
            {
                for (int i = 0; i < config.GetAreaCount(); i++)
                {
                    if (
                        config.ScreenAreas[i].IsEnabled
                        &&
                        config.TabletAreas[i].IsInside(clickPosition)
                        &&

                        // Area smaller than last found area?
                        (
                            area == null
                            ||
                            area.Width > config.TabletAreas[i].Width
                        )
                    )
                    {
                        area = config.TabletAreas[i];
                        config.SelectedScreenArea = config.ScreenAreas[i];
                        config.SelectedTabletArea = config.TabletAreas[i];
                        index = i;
                    }
                }
                if (index == -1)
                {
                    config.SelectedScreenArea = config.ScreenAreas[0];
                    config.SelectedTabletArea = config.TabletAreas[0];
                }
            }

            if (area != null)
            {
                mouseArea.Area = area;
                mouseArea.Index = index;
                mouseArea.IsValid = true;
            }
            else
            {
                mouseArea.Area = null;
                mouseArea.Index = index;
                mouseArea.IsValid = false;
            }
            mouseArea.Point.X = clickPosition.X;
            mouseArea.Point.Y = clickPosition.Y;


        }

        //
        // Update area information
        //
        void UpdateAreaInformation()
        {
            // Update area information label
            int areaIndex = mouseArea.Index;
            if (areaIndex < 0) areaIndex = 0;
            string areaText = "AREA #" + (areaIndex + 1) + " | ";
            if (config.GetEnabledAreaCount() == 1) areaText = "";

            // Screen area
            labelScreenAreaInfo.Content = areaText +
                Utils.GetNumberString(config.SelectedScreenArea.Width / config.SelectedScreenArea.Height, "0.000") + ":1 | " +
                Utils.GetNumberString(config.SelectedScreenArea.Width * config.SelectedScreenArea.Height, "0") + " pixels";

            // Tablet area
            labelTabletAreaInfo.Content = areaText +
                Utils.GetNumberString(config.SelectedTabletArea.Width / config.SelectedTabletArea.Height, "0.000") + ":1 | " +
                Utils.GetNumberString(config.SelectedTabletArea.Width * config.SelectedTabletArea.Height, "0") + " mm² " +
                Utils.GetNumberString(
                    config.SelectedTabletArea.Width * config.SelectedTabletArea.Height /
                    (config.TabletFullArea.Width * config.TabletFullArea.Height) * 100.0
                    , "0") + "% of " +
                Utils.GetNumberString(config.TabletFullArea.Width) + "x" + Utils.GetNumberString(config.TabletFullArea.Height) + " mm | " +
                Utils.GetNumberString(config.SelectedScreenArea.Width / config.SelectedTabletArea.Width, "0.0") + "x" +
                Utils.GetNumberString(config.SelectedScreenArea.Height / config.SelectedTabletArea.Height, "0.0") + " px/mm";

        }

        //
        // Canvas mouse events
        //
        //
        // Canvas mouse down
        private void CanvasArea_MouseDown(object sender, MouseButtonEventArgs e)
        {
            bool mouseDown = false;

            // Is the button left mouse button?
            if (e.LeftButton == MouseButtonState.Pressed)
                mouseDown = true;

            // Update last clicked area
            Point cursorPosition = e.GetPosition((UIElement)sender);
            UpdateMouseArea((UIElement)sender, cursorPosition);

            if (sender != canvasScreenMap && sender != canvasTabletArea) return;

            mouseDrag.Source = (UIElement)sender;
            mouseDrag.OriginMouse = cursorPosition;

            //
            // Screen map drag
            //
            if (mouseDrag.Source == canvasScreenMap)
            {
                if (!mouseArea.IsValid) return;
                mouseDrag.DragArea = mouseArea.Area;

                // Reset monitor selection
                comboBoxMonitor.SelectedIndex = -1;

                mouseDrag.IsMouseDown = mouseDown;
                mouseDrag.OriginDraggable = new Point(mouseDrag.DragArea.X, mouseDrag.DragArea.Y);
                canvasScreenMap.CaptureMouse();
            }

            //
            // Tablet area drag
            //
            else if (mouseDrag.Source == canvasTabletArea)
            {
                if (!mouseArea.IsValid) return;
                mouseDrag.DragArea = mouseArea.Area;

                mouseDrag.IsMouseDown = mouseDown;
                mouseDrag.OriginDraggable = new Point(mouseDrag.DragArea.X, mouseDrag.DragArea.Y);
                canvasTabletArea.CaptureMouse();
            }

        }


        //
        // Canvas mouse up
        //
        private void CanvasArea_MouseUp(object sender, MouseButtonEventArgs e)
        {
            mouseDrag.IsMouseDown = false;
            LoadSettingsFromConfiguration();
            //isLoadingSettings = true;
            textScreenAreaX.Text = Utils.GetNumberString(config.SelectedScreenArea.X - config.SelectedScreenArea.Width / 2.0, "0");
            textScreenAreaY.Text = Utils.GetNumberString(config.SelectedScreenArea.Y - config.SelectedScreenArea.Height / 2.0, "0");
            textTabletAreaX.Text = Utils.GetNumberString(config.SelectedTabletArea.X);
            textTabletAreaY.Text = Utils.GetNumberString(config.SelectedTabletArea.Y);
            //isLoadingSettings = false;
            canvasScreenMap.ReleaseMouseCapture();
            canvasTabletArea.ReleaseMouseCapture();

            // Focus
            if (sender == canvasScreenMap)
                canvasScreenMap.Focus();
            else if (sender == canvasTabletArea)
                canvasTabletArea.Focus();


        }


        //
        // Canvas mouse move
        //
        private void CanvasArea_MouseMove(object sender, MouseEventArgs e)
        {
            Point position;
            double dx, dy;
            double scaleX = 0, scaleY = 0, scale = 0;
            double gridSize = 1;

            // Canvas mouse drag
            if (mouseDrag.IsMouseDown && mouseDrag.Source == sender)
            {
                position = e.GetPosition((UIElement)mouseDrag.Source);

                dx = position.X - mouseDrag.OriginMouse.X;
                dy = position.Y - mouseDrag.OriginMouse.Y;

                // Shift + Drag -> Only move up/down
                if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
                    dx = 0;

                // Control + Drag -> Only move left/right
                if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
                    dy = 0;

                // Alt + Drag -> Use larger grid size
                if (sender == canvasScreenMap && Keyboard.Modifiers.HasFlag(ModifierKeys.Alt))
                    gridSize = 80;
                if (sender == canvasTabletArea && Keyboard.Modifiers.HasFlag(ModifierKeys.Alt))
                    gridSize = 5;


                // Screen map canvas
                if (mouseDrag.Source == canvasScreenMap && mouseDrag.DragArea != null)
                {
                    scaleX = config.DesktopSize.Width / canvasScreenMap.ActualWidth;
                    scaleY = config.DesktopSize.Height / canvasScreenMap.ActualHeight;
                    scale = scaleY;
                    if (scaleX > scaleY)
                        scale = scaleX;

                    // Grid
                    dx = Math.Round(dx * scale / gridSize) * gridSize / scale;
                    dy = Math.Round(dy * scale / gridSize) * gridSize / scale;

                    // Move area
                    mouseDrag.DragArea.X = mouseDrag.OriginDraggable.X + dx * scale;
                    mouseDrag.DragArea.Y = mouseDrag.OriginDraggable.Y + dy * scale;

                    LoadSettingsFromConfiguration();
                }

                // Tablet area canvas
                else if (mouseDrag.Source == canvasTabletArea && mouseDrag.DragArea != null)
                {
                    scaleX = config.TabletFullArea.Width / canvasTabletArea.ActualWidth;
                    scaleY = config.TabletFullArea.Height / canvasTabletArea.ActualHeight;
                    scale = scaleY;
                    if (scaleX > scaleY)
                        scale = scaleX;

                    // Grid
                    dx = Math.Round(dx * scale / gridSize) * gridSize / scale;
                    dy = Math.Round(dy * scale / gridSize) * gridSize / scale;

                    // Move area
                    mouseDrag.DragArea.X = mouseDrag.OriginDraggable.X + dx * scale;
                    mouseDrag.DragArea.Y = mouseDrag.OriginDraggable.Y + dy * scale;

                    LoadSettingsFromConfiguration();
                }
            }

        }


        //
        // Screen or tablet area mouse scroll
        //
        private void CanvasArea_MouseWheel(object sender, MouseWheelEventArgs e)
        {

            //
            // Screen or tablet area scroll
            //
            if (sender == canvasScreenMap || sender == canvasTabletArea)
            {
                if (!mouseArea.IsValid) return;
                double delta = e.Delta / 120.0;
                if (sender == canvasScreenMap)
                {
                    delta *= 10;
                }
                if (Keyboard.Modifiers == ModifierKeys.Control)
                {
                    delta *= 10;
                }

                double oldWidth = mouseArea.Area.Width;
                double oldHeight = mouseArea.Area.Height;
                double newWidth = Math.Round(oldWidth + delta);
                double newHeight = oldHeight / oldWidth * newWidth;
                if (delta > 0 || newWidth >= 10 || newHeight >= 10)
                {
                    mouseArea.Area.Width = newWidth;
                    mouseArea.Area.Height = newHeight;
                }
                LoadSettingsFromConfiguration();
            }

        }


        //
        // Canvas context menu click
        //
        private void CanvasArea_MenuClick(object sender, RoutedEventArgs e)
        {
            if (sender == null) return;
            if (!(sender is MenuItem)) return;
            MenuItem menuItem = (MenuItem)sender;


            //
            // Move screen area center of a monitor or set to full monitor
            //
            if (sender == menuCanvasMoveToMonitorCenter || sender == menuCanvasSetToFullMonitor)
            {
                if (!mouseArea.IsValid) return;

                // Get monitors
                System.Windows.Forms.Screen[] screens = GetAvailableScreens();
                Vector minimumScreenPosition = GetMinimumScreenPosition(screens);

                // Loop through monitors
                foreach (System.Windows.Forms.Screen screen in screens)
                {
                    double x = screen.Bounds.X - minimumScreenPosition.X;
                    double y = screen.Bounds.Y - minimumScreenPosition.Y;
                    if (
                        mouseArea.Point.X >= x && mouseArea.Point.X <= x + screen.Bounds.Width
                        &&
                        mouseArea.Point.Y >= y && mouseArea.Point.Y <= y + screen.Bounds.Height
                    )
                    {
                        mouseArea.Area.X = x + screen.Bounds.Width / 2.0;
                        mouseArea.Area.Y = y + screen.Bounds.Height / 2.0;
                        if (sender == menuCanvasSetToFullMonitor)
                        {
                            mouseArea.Area.Width = screen.Bounds.Width;
                            mouseArea.Area.Height = screen.Bounds.Height;
                        }
                        LoadSettingsFromConfiguration();
                        break;
                    }
                }

            }


            //
            // Set to full screen area
            //
            else if (sender == menuCanvasSetToFullDesktop)
            {
                if (!mouseArea.IsValid) return;
                mouseArea.Area.X = config.DesktopSize.X;
                mouseArea.Area.Y = config.DesktopSize.Y;
                mouseArea.Area.Width = config.DesktopSize.Width;
                mouseArea.Area.Height = config.DesktopSize.Height;

                LoadSettingsFromConfiguration();
            }


            //
            // Move tablet area to center
            //
            else if (sender == menuCanvasMoveToCenter)
            {
                if (!mouseArea.IsValid) return;

                mouseArea.Area.X = config.TabletFullArea.Width / 2.0;
                mouseArea.Area.Y = config.TabletFullArea.Height / 2.0;

                LoadSettingsFromConfiguration();
            }


            //
            // Set to full tablet area
            //
            else if (sender == menuCanvasSetToFullTabletArea)
            {
                if (!mouseArea.IsValid) return;
                mouseArea.Area.Width = config.TabletFullArea.Width;
                mouseArea.Area.Height = config.TabletFullArea.Height;
                mouseArea.Area.X = config.TabletFullArea.X;
                mouseArea.Area.Y = config.TabletFullArea.Y;
                FixTabletAreaDimensions(config.SelectedTabletArea, config.SelectedScreenArea);
                LoadSettingsFromConfiguration();
            }

            //
            // Force tablet area proportions
            //
            else if (sender == menuCanvasForceProportions)
            {
                double screenAspectRatio = config.SelectedScreenArea.Width / config.SelectedScreenArea.Height;
                double tabletAspectRatio = config.SelectedTabletArea.Width / config.SelectedTabletArea.Height;

                // Tablet aspect ratio higher -> change width
                if (tabletAspectRatio >= screenAspectRatio)
                    config.SelectedTabletArea.Width = config.SelectedTabletArea.Height * screenAspectRatio;

                // Tablet aspect ratio lower -> change  height
                else
                    config.SelectedTabletArea.Height = config.SelectedTabletArea.Width / screenAspectRatio;

                LoadSettingsFromConfiguration();
            }

            //
            // Add secondary area
            //
            else if (sender == menuCanvasAddScreenArea || sender == menuCanvasAddTabletArea)
            {
                for (int i = 1; i < config.GetAreaCount(); i++)
                {
                    // Area is not enabled?
                    if (!config.ScreenAreas[i].IsEnabled)
                    {
                        // Enable area
                        config.ScreenAreas[i].IsEnabled = true;

                        // Screen area
                        if (sender == menuCanvasAddScreenArea)
                        {
                            config.ScreenAreas[i].X = mouseArea.Point.X;
                            config.ScreenAreas[i].Y = mouseArea.Point.Y;
                        }
                        else
                        {
                            config.ScreenAreas[i].X = config.DesktopSize.Width / 2.0;
                            config.ScreenAreas[i].Y = config.DesktopSize.Height / 2.0;
                        }
                        config.ScreenAreas[i].Width = 640;
                        config.ScreenAreas[i].Height = 640 * config.ScreenAreas[0].Height / config.ScreenAreas[0].Width;

                        // Tablet area
                        if (sender == menuCanvasAddTabletArea)
                        {
                            config.TabletAreas[i].X = mouseArea.Point.X;
                            config.TabletAreas[i].Y = mouseArea.Point.Y;
                        }
                        else
                        {
                            config.TabletAreas[i].X = config.TabletFullArea.Width / 2.0;
                            config.TabletAreas[i].Y = config.TabletFullArea.Height / 2.0;
                        }
                        config.TabletAreas[i].Width = 50;
                        config.TabletAreas[i].Height = 50 * config.ScreenAreas[0].Height / config.ScreenAreas[0].Width;

                        break;
                    }
                }
                LoadSettingsFromConfiguration();
            }

            //
            // Remove area
            //
            else if (sender == menuCanvasRemoveScreenArea || sender == menuCanvasRemoveTabletArea)
            {
                if (!mouseArea.IsValid) return;

                WindowMessageBox messageBox = new WindowMessageBox(
                    "Are you sure?", "Remove area #" + (mouseArea.Index + 1) + "?",
                    "Yes", "No");
                messageBox.ShowDialog();
                if (messageBox.DialogResult == true)
                {
                    bool removed = false;
                    for (int i = 1; i < config.GetAreaCount(); i++)
                    {
                        // Item to be removed found
                        if (!removed && config.ScreenAreas[i] == mouseArea.Area || config.TabletAreas[i] == mouseArea.Area)
                        {
                            removed = true;
                        }

                        // Shift area arrays
                        else if (removed)
                        {
                            config.ScreenAreas[i - 1].Set(config.ScreenAreas[i]);
                            config.TabletAreas[i - 1].Set(config.TabletAreas[i]);
                            config.ScreenAreas[i].IsEnabled = false;
                            config.TabletAreas[i].IsEnabled = false;
                        }
                    }

                    // Select primary area
                    if (removed)
                    {
                        config.SelectedScreenArea = config.ScreenAreas[0];
                        config.SelectedTabletArea = config.TabletAreas[0];
                        mouseArea.Area = null;
                        mouseArea.IsValid = false;
                        LoadSettingsFromConfiguration();
                    }
                }
            }


            //
            // Edit area
            //           
            else if (sender == menuCanvasEditScreenArea || sender == menuCanvasEditTabletArea)
            {
                if (!mouseArea.IsValid) return;

                int areaIndex = -1;
                for (int i = 0; i < config.GetAreaCount(); i++)
                {
                    if (config.ScreenAreas[i] == mouseArea.Area || config.TabletAreas[i] == mouseArea.Area)
                    {
                        areaIndex = i;
                    }
                }
                if (areaIndex >= 0)
                {
                    WindowAreaEditor areaEditor = new WindowAreaEditor(config, config.ScreenAreas[areaIndex], config.TabletAreas[areaIndex])
                    {
                        Title = "Area #" + (areaIndex + 1)
                    };
                    areaEditor.ShowDialog();
                    if (areaEditor.DialogResult == true)
                    {
                        LoadSettingsFromConfiguration();
                    }
                }
            }


            //
            // Reset screen areas
            //
            else if (sender == menuCanvasResetScreenAreas)
            {
                WindowMessageBox messageBox = new WindowMessageBox(
                                           "Are you sure?", "Reset all screen areas?",
                                           "Yes", "No");
                messageBox.ShowDialog();
                if (messageBox.DialogResult == true)
                {
                    double offsetY = -(config.GetEnabledAreaCount() * 100.0 / 2.0);
                    foreach (Area screenArea in config.ScreenAreas)
                    {
                        screenArea.X = config.DesktopSize.X;
                        screenArea.Y = config.DesktopSize.Y + offsetY;
                        screenArea.Width = config.DesktopSize.Width / 2.0;
                        screenArea.Height = config.DesktopSize.Height / 2.0;
                        offsetY += 100;
                    }
                    LoadSettingsFromConfiguration();
                }
            }


            //
            // Reset tablet areas
            //
            else if (sender == menuCanvasResetTabletAreas)
            {
                WindowMessageBox messageBox = new WindowMessageBox(
                              "Are you sure?", "Reset all tablet areas?",
                              "Yes", "No");
                messageBox.ShowDialog();
                if (messageBox.DialogResult == true)
                {
                    double offsetY = -(config.GetEnabledAreaCount() * 10.0 / 2.0);
                    foreach (Area tabletArea in config.TabletAreas)
                    {
                        tabletArea.X = config.TabletFullArea.X;
                        tabletArea.Y = config.TabletFullArea.Y + offsetY;
                        tabletArea.Width = config.TabletFullArea.Width / 2.0;
                        tabletArea.Height = config.TabletFullArea.Height / 2.0;
                        offsetY += 10.0;

                    }
                    LoadSettingsFromConfiguration();
                }
            }


        }


        //
        // Canvas context menu opening
        //
        private void CanvasArea_ContextMenuOpening(object sender, ContextMenuEventArgs e)
        {

            //
            // Screen area canvas context menu
            //
            if (sender == canvasScreenMap)
            {
                if (!mouseArea.IsValid)
                {
                    foreach (object item in canvasScreenMap.ContextMenu.Items)
                        ((UIElement)item).Visibility = Visibility.Collapsed;
                    menuCanvasResetScreenAreas.Visibility = Visibility.Visible;
                }
                else
                {
                    // Find area index
                    int index = 0;
                    for (int i = 0; i < config.GetAreaCount(); i++)
                    {
                        if (config.ScreenAreas[i] == mouseArea.Area) index = i;
                    }

                    // Create area information string
                    string areaInfo = "Area #" + (index + 1) + ": ";
                    areaInfo += Utils.GetNumberString(mouseArea.Area.Width, "0") + "x" +
                        Utils.GetNumberString(mouseArea.Area.Height, "0") + ", ";
                    areaInfo += Utils.GetNumberString(mouseArea.Area.Width / mouseArea.Area.Height, "0.000") + ":1";

                    menuCanvasScreenAreaInfo.Header = areaInfo;
                    foreach (object item in canvasScreenMap.ContextMenu.Items)
                        ((UIElement)item).Visibility = Visibility.Visible;

                    if (index == 0)
                    {
                        menuCanvasRemoveScreenArea.Visibility = Visibility.Collapsed;
                    }


                }

                // Hide add secondary area menu item
                int areaCount = 0;
                foreach (Area area in config.ScreenAreas)
                {
                    if (area.IsEnabled) areaCount++;
                }
                if (areaCount < config.GetAreaCount())
                    menuCanvasAddScreenArea.Visibility = Visibility.Visible;
                else
                    menuCanvasAddScreenArea.Visibility = Visibility.Collapsed;
            }

            //
            // Tablet area canvas context menu
            //
            else if (sender == canvasTabletArea)
            {
                if (!mouseArea.IsValid)
                {
                    foreach (object item in canvasTabletArea.ContextMenu.Items)
                        ((UIElement)item).Visibility = Visibility.Collapsed;
                    menuCanvasAddTabletArea.Visibility = Visibility.Visible;
                    menuCanvasResetTabletAreas.Visibility = Visibility.Visible;

                }
                else
                {
                    // Find area index
                    int index = 0;
                    for (int i = 0; i < config.GetAreaCount(); i++)
                    {
                        if (config.TabletAreas[i] == mouseArea.Area) index = i;
                    }

                    // Create area information string
                    string areaInfo = "Area #" + (index + 1) + ": ";
                    areaInfo += Utils.GetNumberString(mouseArea.Area.Width) + "mm x " +
                        Utils.GetNumberString(mouseArea.Area.Height) + "mm, ";
                    areaInfo += Utils.GetNumberString(mouseArea.Area.Width / mouseArea.Area.Height, "0.000") + ":1";

                    menuCanvasTabletAreaInfo.Header = areaInfo;

                    foreach (object item in canvasTabletArea.ContextMenu.Items)
                        ((UIElement)item).Visibility = Visibility.Visible;

                    if (index == 0)
                    {
                        menuCanvasRemoveTabletArea.Visibility = Visibility.Collapsed;
                    }
                }

                // Hide add secondary area menu item
                int areaCount = 0;
                foreach (Area area in config.ScreenAreas)
                {
                    if (area.IsEnabled) areaCount++;
                }
                if (areaCount < config.GetAreaCount())
                    menuCanvasAddTabletArea.Visibility = Visibility.Visible;
                else
                    menuCanvasAddTabletArea.Visibility = Visibility.Collapsed;

            }

        }



        #region Wacom / Draw area

        //
        // Wacom Area
        //
        private void ButtonWacomArea_Click(object sender, RoutedEventArgs e)
        {
            WindowWacomArea wacom = new WindowWacomArea(config, config.SelectedTabletArea);
            wacom.ShowDialog();

            // Set button clicked
            if (wacom.DialogResult == true)
            {
                LoadSettingsFromConfiguration();
            }
        }


        //
        // Draw area
        //
        private void ButtonDrawArea_Click(object sender, RoutedEventArgs e)
        {
            if (!isEnabledMeasurementToArea)
            {
                isEnabledMeasurementToArea = true;
                driver.SendCommand("Measure 2");
                SetStatus("Click the top left and the bottom right corners of the area with your tablet pen!");
                buttonDrawArea.IsEnabled = false;
            }
        }

        #endregion

    }
}
