Add a new Element type to the dynamic Controller
================================================

The dynamic controller component (DCC) supports different element types,
like ``button``, ``linechart`` and ``switch``.
In this example we will add the new element type ``label`` to this list.

This simple element will display a single labeled information like this:

.. image:: ../label.png

So beside an id and the type, we need to pass a title and a source for the
value via the DCC JSON specification:

.. code-block:: json

    {
        "id": "l-targetspeed",
        "type": "label",
        "title": "Target Speed (km/h)",
        "source": {
            "endpoint": "/$CONTROLLER/state",
            "path": "target_speed"
        }
    }

If Cloe UI loads this specification, it will raise an error because the type
``label`` is unknown yet. So let's implement it!

Preparation
-----------

The prerequisite for a new element type is that a corresponding react component
exists. It is possible to define everything directly in the DCC, but in terms
of reusability and being 'React-ish', you should define it as an
individual component.

So we define a new react component named ``Label`` in a new file
``src/components/label.jsx`` :

.. code-block:: jsx
    :linenos:

    // The Label component represents one single key-value pair.
    // It displays a value with it's key (label).

    import React from "react";

    function Label(props) {
        const { label, value } = props;
        return (
            <div className="container">
                <div className="row">
                    <div className="col-sm">
                        <small className="font-weight-light text-secondary">
                            {label}
                        </small>
                    </div>
                </div>
                <div className="row">
                    <div className="col-sm">
                        <span>
                            {value}
                        </span>
                    </div>
                </div>
            </div>
        );
    }

    export default Label;

Our component takes two props, ``label`` and ``value`` (line 7) and
displays them in a bootstrap grid.

Implementation in DCC
---------------------

Now that the component is defined by itself, we can use it in the DCC.
The DCC is defined as a react class component called ``Controller`` in
``src/components/controller.jsx``.

The first step is to import our ``Label`` component outside of the
``Controller`` class:

.. code-block:: javascript

    import Label from "./label";

For each element type, the class has a method which takes the specified
information (in our case mainly the title and source), and pass them over
to the right Component. This methods name's are prefixed with ``generate``,
e.g. the method to generate a button is named ``generateButton``.

So as second step, we add a new method ``generateLabel`` to the
``Controller`` class, which takes a ``labelElement`` as argument:

.. code-block:: jsx

    generateLabel = (labelElement) => {};

The ``labelElement`` will be an object which includes all relevant information
which is needed to render it. It's basic structure is similar to it's JSON
specification. The only differences are in the ``sources`` property
and the new ``reactElement`` property. It looks like this:

.. code-block:: javascript
    :linenos:

    {
        "id": "l-targetspeed",
        "title": "Target Speed (km/h)",
        "type": "label"
        "sources": [{
            "endpoint": "/$CONTROLLER/state",
            "path": "target_speed",
            "math": undefined,
            "name": undefined
        }]
        "reactElement": undefined
    }

The yet undefined ``reactElement`` is what is needed by Cloe UI to render
something. We will generate it with our ``generateLabel`` method.

So we need to pass the ``label`` and the correseponding value to the
``Label`` component.

The ``label`` is easy, we will take the ``labelElement.title`` property.
To get the value, we use a method called ``_genSourceValue(source)``.
Since ``labelElement.sources`` is of type array, we need to pass
``labelElement.sources[0]`` as argument:

.. code-block:: jsx
    :linenos:

    generateLabel = (labelElement) => {
        const value = this._getSourceValue(labelElement.sources[0]);
        const title = labelElement.title;
    };

Now we can use our ``Label`` component, pass all needed information and
store it in ``labelElement.reactElement`` and return the updated element:

.. code-block:: jsx
    :linenos:

    generateLabel = (labelElement) => {
        const value = this._getSourceValue(labelElement.sources[0]);
        const title = labelElement.title;

        labelElement.reactElement = (
            <Label
                key={labelElement.id}
                label={title}
                value={value}/>
        );
        return labelElement;
    };

.. note::

    Make sure to specify a unique key prop. This is needed by react in
    order to differ between elements based on the same component.
    the element's id should be fine in most of the cases.

The third and final step is to make sure that ``Controller`` identifies
the new element type and execute the ``generateLabel`` method if needed.

The ``generate`` methods are executed from within the ``generateJSX`` method.
It iterates through all specified elements, identifies their types and executes
the respective ``generate`` method. This logic is implemented as an switch-case
construct.

We simply have to expand it with our new type and its generator method:

.. code-block:: javascript
    :linenos:

    generateJSX = () => {
        for (let index in this.elements) {
            switch (this.elements[index].type) {
                case "button":
                    this.elements[index] = this.generateButton(this.elements[index]);
                    break;
                case "switch":
                    this.elements[index] = this.generateSwitch(this.elements[index]);
                    break;
                case "label":
                    this.elements[index] = this.generateLabel(this.elements[index]);
                    break;
                default:
                    break
            }
        }
    };
