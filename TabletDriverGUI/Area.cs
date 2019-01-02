using System;
using System.Windows;

namespace TabletDriverGUI
{
    public class Area
    {
        private double _width;
        private double _height;

        public double Width
        {
            get
            {
                return _width;
            }
            set
            {
                _width = value;
                UpdateCorners();

            }
        }
        public double Height
        {
            get
            {
                return _height;
            }
            set
            {
                _height = value;
                UpdateCorners();

            }
        }
        public double X;
        public double Y;
        public double Rotation
        {
            get { return _rotation; }
            set
            {
                double angle;
                _rotation = value;

                angle = _rotation * Math.PI / 180;

                _rotationMatrix[0] = Math.Cos(angle);
                _rotationMatrix[1] = Math.Sin(angle);
                _rotationMatrix[2] = -Math.Sin(angle);
                _rotationMatrix[3] = Math.Cos(angle);

                UpdateCorners();
            }
        }
        public bool IsEnabled;



        private double _rotation;
        private readonly double[] _rotationMatrix;
       private readonly Point[] _corners;


        //
        // Constructors
        //
        public Area()
        {
            _corners = new Point[4] {
                new Point(0,0),
                new Point(0,0),
                new Point(0,0),
                new Point(0,0)
            };
            _rotation = 0;
            _rotationMatrix = new double[4] { 1, 0, 0, 1 };
            _width = 0;
            _height = 0;
            X = 0;
            Y = 0;
            IsEnabled = false;

        }
        public Area(double width, double height, double x, double y) : this()
        {
            _width = width;
            _height = height;
            X = x;
            Y = y;
            IsEnabled = false;
            UpdateCorners();
        }

        //
        // Copy values from an another area
        //
        public void Set(Area area)
        {
            X = area.X;
            Y = area.Y;
            Width = area.Width;
            Height = area.Height;
            Rotation = area.Rotation;
            IsEnabled = area.IsEnabled;
        }

        //
        // Update corner positions
        //
        private void UpdateCorners()
        {
            GetRotatedPoint(ref _corners[0], -_width / 2.0, -_height / 2.0);
            GetRotatedPoint(ref _corners[1], _width / 2.0, -_height / 2.0);
            GetRotatedPoint(ref _corners[2], _width / 2.0, _height / 2.0);
            GetRotatedPoint(ref _corners[3], -_width / 2.0, _height / 2.0);
        }

        //
        // Rotate point
        //
        public void GetRotatedPoint(ref Point p, double x, double y)
        {
            p.X = x * _rotationMatrix[0] + y * _rotationMatrix[1];
            p.Y = x * _rotationMatrix[2] + y * _rotationMatrix[3];
        }

        //
        // Rotate point in reverse direction
        //
        public void GetRotatedPointReverse(ref Point p, double x, double y)
        {
            p.X = x * _rotationMatrix[3] - y * _rotationMatrix[1];
            p.Y = x * -_rotationMatrix[2] + y * _rotationMatrix[0];
        }


        //
        // Check if a point is inside of the area
        //
        public bool IsInside(Point p)
        {
            double x1, y1, x2, y2;

            x1 = p.X - X;
            y1 = p.Y - Y;
            x2 = x1 * _rotationMatrix[0] + y1 * _rotationMatrix[1];
            y2 = x1 * _rotationMatrix[2] + y1 * _rotationMatrix[3];

            if (
                x2 > -Width / 2.0 &&
                x2 < +Width / 2.0 &&
                y2 > -Height / 2.0 &&
                y2 < +Height / 2.0
            )
            {
                return true;
            }

            return false;
        }

        //
        // Get corners
        //
        public Point[] Corners => _corners;

        //
        // Bounding Box
        //
        public double[] GetBoundingBox()
        {
            double[] box = new double[4] {
                999999,-99999,
                999999,-99999
            };
            foreach (Point p in _corners)
            {
                if (p.X < box[0]) box[0] = p.X;
                if (p.X > box[1]) box[1] = p.X;
                if (p.Y < box[2]) box[2] = p.Y;
                if (p.Y > box[3]) box[3] = p.Y;

            }

            return box;
        }


        //
        // Scale to fit inside another area
        //
        public void ScaleInside(Area target)
        {
            double scale;
            double[] box = GetBoundingBox();

            scale = Math.Max(
                Math.Abs(box[1] * 2 / target.Width),
                Math.Abs(box[3] * 2 / target.Height)
            );

            _width /= scale;
            _height /= scale;
            UpdateCorners();
        }

        //
        // Move area position inside of another area
        //
        public void MoveInside(Area target)
        {
            double[] box = GetBoundingBox();
            double[] targetBox = target.GetBoundingBox();
            if (X + box[0] < target.X + targetBox[0])
                X = target.X + targetBox[0] - box[0];
            if (X + box[1] > target.X + targetBox[1])
                X = target.X + targetBox[1] - box[1];

            if (Y + box[2] < target.Y + targetBox[2])
                Y = target.Y + targetBox[2] - box[2];
            if (Y + box[3] > target.Y + targetBox[3])
                Y = target.Y + targetBox[3] - box[3];
        }


        //
        // Convert to string
        //
        public override string ToString()
        {
            return
                "Area[" +
                    Utils.GetNumberString(X) + "," +
                    Utils.GetNumberString(Y) + "," +
                    Utils.GetNumberString(Width) + "," +
                    Utils.GetNumberString(Height) +
                "]";
        }

    }
}
