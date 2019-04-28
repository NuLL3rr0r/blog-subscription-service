/**
 * @file
 * @author  Mamadou Babaei <info@babaei.net>
 * @version 0.1.0
 *
 * @section LICENSE
 *
 * (The MIT License)
 *
 * Copyright (c) 2016 - 2019 Mamadou Babaei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * Gulp js code to build assets.
 */


var gulp = require('gulp');
var autoprefixer = require('gulp-autoprefixer');
var minifyHtml = require('gulp-minify-html');
var imagemin = require('gulp-imagemin');
var minifyCss = require('gulp-minify-css');
var sass = require('gulp-sass');
var uglify = require('gulp-uglify');

gulp.task('fonts', function(done) {
    gulp.src('fonts/**/*')
        .pipe(imagemin({
        })).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/fonts/')
    );

    done();
});

gulp.task('images', function(done) {
    gulp.src('favicon.ico')
        .pipe(gulp.dest('./output/www/')
    );
//    gulp.src('favicon.png')
//        .pipe(imagemin({
//            optimizationLevel: 7
//        })).on('error', errorHandler)
//        .pipe(gulp.dest('./output/www/')
//    );
    gulp.src('images/**/*.{gif,jpg,png,svg}')
        .pipe(imagemin({
            multipass: true,
            optimizationLevel: 7,
            progressive: true,
        })).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/img/')
    );

    done();
});

gulp.task('scripts', function(done) {
    gulp.src(['javascripts/**/*.js', '!javascripts/**/*.min.js'])
        .pipe(uglify()).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/js/')
    );
    gulp.src(['javascripts/**/*.min.js'])
        .pipe(gulp.dest('./output/www/js/')
    );

    done();
});

gulp.task('styles', function(done) {
    gulp.src('stylesheets/**/*.scss')
        .pipe(sass()).on('error', errorHandler)
        .pipe(autoprefixer('last 2 versions')).on('error', errorHandler)
        .pipe(gulp.dest('./output/css-temp/'))
        .pipe(minifyCss()).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/css/')
    );
    gulp.src('stylesheets/**/*.css')
        .pipe(autoprefixer('last 2 versions')).on('error', errorHandler)
        .pipe(gulp.dest('./output/css-temp/'))
        .pipe(minifyCss()).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/css/')
    );

    done();
});

gulp.task('templates', function() {
    gulp.src('templates/**/*.wtml')
        .pipe(minifyHtml()).on('error', errorHandler)
        .pipe(gulp.dest('./output/templates/')
    );

    done();
});

gulp.task('wt-resources', function(done) {
    // Images
    gulp.src('resources/**/*.{gif,jpg,png,svg}')
        .pipe(imagemin({
            multipass: true,
            optimizationLevel: 7,
            progressive: true,
        })).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/resources/')
    );

    // Scripts
    gulp.src(['resources/**/*.js', '!resources/**/*.min.js'])
        .pipe(uglify()).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/resources/')
    );

    // Styles
    gulp.src(['resources/**/*.css', '!resources/**/*.min.css'])
        .pipe(minifyCss()).on('error', errorHandler)
        .pipe(gulp.dest('./output/www/resources/')
    );

    // The rest of the files without overwriting the optimized ones
    gulp.src(['resources/**/*',
              '!resources/**/*.{gif,jpg,png,svg}',
              '!resources/**/*.js',
              '!resources/**/*.css'])
        .pipe(gulp.dest('./output/www/resources/')
    );
    gulp.src(['resources/**/*.min.css', 'resources/**/*.min.js'])
        .pipe(gulp.dest('./output/www/resources/')
    );

    done();
});

// do not add 'templates', because minifyHtml will mess up all HTML attributes with a variable value (e.g. ${})
gulp.task('default', gulp.parallel('scripts', 'styles', 'fonts', 'images', 'wt-resources'));

// Public error handler
function errorHandler (error) {
  console.log(error.toString());
  this.emit('end');
}
