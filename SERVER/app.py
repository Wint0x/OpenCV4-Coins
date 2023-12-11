import os
from call_opencv import CallCommand
from flask import Flask, flash, request, redirect, url_for, render_template, make_response
from werkzeug.utils import secure_filename


UPLOAD_FOLDER = 'static/Image'
ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'gif'}

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.secret_key = b'_5#y2L"F4Q8z\n\xec]/'

def allowed_file(filename):
    return '.' in filename and \
    filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route("/")
def hello_world():

    return """<p>Hello, World!</p>\n<h1><a href=\"upload\">Upload a file!</a></h1>"""

@app.route('/upload', methods=['GET'])
def upload():
	error = None

	return render_template('upload.html', name=None, error=error)

@app.route('/get_image', methods=['GET', 'POST'])
def upload_file():

    if request.method == 'POST':
        # check if the post request has the file part
        if 'file' not in request.files:
            flash('No file part')
            return redirect(request.url)

        file = request.files['file']

        mode = request.form.get('mode')
        mode = ('1' if mode == "hough" else '2')

        print("[DEBUG] -> MODE: ", mode)

        # If the user does not select a file, the browser submits an
        # empty file without a filename.
        if file.filename == '':
            flash('No selected file')
            return redirect(request.url)

        if file and allowed_file(file.filename):

            filename = secure_filename(file.filename)

            saved_img = os.path.join(app.config['UPLOAD_FOLDER'], filename)

            file.save(saved_img)
            
            # Analyze with Open-Cv
            analyze_image = CallCommand(saved_img, mode=mode)
            
            identifier = analyze_image.check_out()

            return render_template('image.html', identifier = identifier, filename=filename)

    return '''
    <!doctype html>
    <title>Upload new File</title>
    <h1>Upload new File</h1>
    <form method=post enctype=multipart/form-data>
      <input type=file name=file>
      <input type=submit value=Upload>
    </form>
    '''

if __name__ == '__main__':
	app.run()