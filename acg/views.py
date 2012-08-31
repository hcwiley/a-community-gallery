from datetime import datetime
from django import http
from django.core.context_processors import csrf
from django.http import Http404
from django.shortcuts import render_to_response, Http404, HttpResponse
from django.template import Context, Template, RequestContext, loader
from django.views.generic.simple import redirect_to
from django.views.decorators.csrf import requires_csrf_token
from acg import settings
from piece.models import *
#from biz.models import BusinessDetail, Association, Service, Link
#from contactform.utils import submit as submit_contact_form
#from contactform.utils import validation as contact_validation
#from contactform.forms import ContactForm

pages = ['home', 'about', 'gallery', 'upload', 'tv']

def common_args(ajax = False):
    """
    The common arguments for all django views.
    ajax: Describes if the args are for an ajax request.
    
    STATIC_URL: static url from settings
    year: the year at the time of request
    contact: django business contact information
    base_template: the default base template  
    """
    args = {
               'STATIC_URL' : settings.STATIC_URL,
               'year' : datetime.now().year,
               'piece_form' : PieceForm(),
               'base_template' : 'base.html',
               'html' : '<h3> Oh this is happening </h3>',
               'IS_DEV': settings.IS_DEV
           }
    if ajax:
        args['base_template'] = "base-ajax.html"
    return args

# This can be called when CsrfViewMiddleware.process_view has not run, therefore
# need @requires_csrf_token in case the template needs {% csrf_token %}.
@requires_csrf_token
def gallery_404(request, template_name = 'common/404.html'):
    """ 
    404 handler for django.

    Templates: `404.html`
    Context:
        common_args from django
        request_path
            The path of the requested URL (e.g., '/app/pages/bad_page/')
        
    """
    t = loader.get_template(template_name) # You need to create a 404.html template.
    args = common_args()
    args['request_path'] = request.path
    return http.HttpResponseNotFound(t.render(RequestContext(request, args)))

@requires_csrf_token
def gallery_500(request, template_name = '500.html'):
    """ 
    500 error handler for django.

    Templates: `500.html`
    Context: common_args from django
    """
    t = loader.get_template(template_name) # You need to create a 500.html template.
    return http.HttpResponseServerError(t.render(Context(common_args())))

def xml(request):
    args = common_args(False)
    args.update(csrf(request))
    pieces = Piece.objects.all()
    args.update({'pieces': pieces}) 
    return render_to_response('xml.txt', args)

def default(request, page):
    args = common_args(False)
    args.update(csrf(request))
    if page == '':
        page = 'home'
    if page == 'gallery' or page=='tv':
        pieces = Piece.objects.all()
        args.update({'pieces': pieces}) 
    if page in pages:
        return render_to_response('%s.html' % page, args)
    else:
        raise Http404

def ajax(request, page):
    print 'ajax'
    #    print request
    print page
    args = common_args(True)
    args.update(csrf(request))
    if page == '' or page == '/' :
        page = 'home'
    if page == 'gallery':
        pieces = Piece.objects.all()
        args.update({'pieces': pieces})
    if page in pages:
        return render_to_response('%s.html' % page, args)
    else:
        raise Http404


def home(request, ajax):
    """
    Renders the home page.
    Context:
        common args
        assocs - list of all professional associations
    """
    args = common_args(ajax)
    args.update(csrf(request))
    return render_to_response('home.html', args)

def remove_slash(request, url):
    """
    Rechecks the URL without the trailing slash(es) before raising an Http404.
    TODO: look into moving this into the custom 404 handler - won't need have a catchall url this way.
    """
    if url.endswith('/'):
        return redirect_to(request, '/' + url.rstrip('/'))
    else:
        raise Http404

def admin_add_slash(request):
    """
    Because APPEND_SLASH is false, manually append slash in this case.
    """
    return redirect_to(request, request.path + '/')

def add_work(request):
    if request.method != "POST":
        raise Http404
    img = request.FILES['default_image']
    img = MyImage.objects.create(image=img)
    img.save()
    form = PieceForm(request.POST)
    if form.is_valid():
        piece = form.save(commit=False)
        piece.default_image = img
        piece.save()
        args = common_args()
        args.update({"html" : "<h3>I've received your submission</h3>"})
        return render_to_response("response.html", args)
    else:
        args = common_args()
        args.update({"html" : "<h3>So something got messed up<br>I shall now list all the things</h3><br><br>%s" % form.errors})
        return render_to_response("response.html", args)
